#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include "cell_writer_iso8211dirty.h"
#include "cell_s57_base.h"
#include "cell_records.h"
#include "iso8211_simple.h"

using namespace Enc;

#include "cell_writer_iso8211dirtyDDR.incl"
#include "cell_records.h"

//*****************************************************************************
/// 
/*!
****************************************************************************** */
CellWriter8211Dirty::~CellWriter8211Dirty()
{}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
void CellWriter8211Dirty::writeS57Cell(QIODevice * cellOutputDevice)
{
    if (!cellS57) throw QString("ERROR: Cannot write S-57 Cell: No Cell Pointer!");
    if (!cellOutputDevice) throw QString("ERROR: Cannot write S-57 Cell: No Ouput Device!");

    cellDev = cellOutputDevice;

    unsigned long bytesWritten = 0;
    currentIso8211rcid = 1;

    if (cellS57->recordCount() >= 0xFFFE) field0001len = 4;  //this is against the S-57 standard !!!
    else field0001len = 2;

    //****** write the DDR ******
    //** if the DDR of the source cell was is stored, use it again **
    if (cellS57->DDR)
    {
        ulong ddrBytes = cellDev->write(cellS57->DDR, cellS57->DDRlen); 
        if (ddrBytes != cellS57->DDRlen) throw QString("ERROR writing DDR to file!"); //maybe there are strange chars in DDR, who knows!
        bytesWritten += ddrBytes;
    }
    //** write the default S-57 DDR **
    else
    {
        bytesWritten += cellDev->write(DefaultDDR, strlen(DefaultDDR)); 
    }
    currentIso8211rcid = 2;

    //****** write the CellHeader records ******
    bytesWritten += writeDSGIrecord();
    ++currentIso8211rcid;

    //** rem: Updates have NO DSGR record (DSPM field) **
    if (cellS57->getDsid().getPROF() == 1)
    {
        bytesWritten += writeDSGRrecord();
        ++currentIso8211rcid;
    }

    //****** write vector records ******
    const std::map< unsigned long, NodeS57 *> & iNodes = cellS57->getINodes();  
    std::map< unsigned long, NodeS57 *>::const_iterator inIt;
    for(inIt = iNodes.begin(); inIt != iNodes.end(); ++inIt)
    {
        bytesWritten += writeNode(*inIt->second);
        ++currentIso8211rcid;
    }
    const std::map< unsigned long, BoundNodeS57 *> & bNodes = cellS57->getBNodes(); 
    std::map< unsigned long, BoundNodeS57 *>::const_iterator biIt;
    for(biIt = bNodes.begin(); biIt != bNodes.end(); ++biIt)
    {
        bytesWritten += writeNode(*biIt->second);
        ++currentIso8211rcid;
    }

    const std::map< unsigned long, SoundgS57 *> & soundings = cellS57->getSoundingds();
    std::map< unsigned long, SoundgS57 *>::const_iterator snIt;
    for(snIt = soundings.begin(); snIt != soundings.end(); ++snIt)
    {
        bytesWritten += writeSounding(*snIt->second);
        ++currentIso8211rcid;
    }
    const std::map< unsigned long, EdgeS57 *> & edges = cellS57->getEdges(); 
    std::map< unsigned long, EdgeS57 *>::const_iterator edIt;
    for(edIt = edges.begin(); edIt!= edges.end(); ++edIt)
    {
        bytesWritten += writeEdge(*edIt->second);
        ++currentIso8211rcid;
    }

    //****** write feature records ******
    const std::map< unsigned long, FeatureS57 *> & features = cellS57->getFeatures();
    std::map< unsigned long, FeatureS57 *>::const_iterator feIt;
    for(feIt = features.begin(); feIt != features.end(); ++feIt)
    {
        bytesWritten += writeFeature(*feIt->second);
        ++currentIso8211rcid;
    }    
}

//*****************************************************************************
/// Write the CellHeader Record (DSGI) 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeDSGIrecord()
{
    unsigned long bytesWritten = 0;
    
    //****** build the directory ******
    RecordLeaderDirWriter dirWriter(field0001len +1); 
    dirWriter.addField("DSID", cellS57->getDsid().getIso8211Length());    
    dirWriter.addField("DSSI", cellS57->getDssi().getIso8211Length());    
    bytesWritten += dirWriter.write(cellDev);

    //****** write the 0001 field ******
    bytesWritten += ISO8211::write0001fieldBIN(cellDev, currentIso8211rcid, field0001len +1);
    //****** write the DSID field ******
    bytesWritten += writeDSIDField();
    //****** write the DSSI field ******
    bytesWritten += writeDSSIField();

    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeDSGRrecord()
{
    unsigned long bytesWritten = 0;

    //****** build the directory ******
    RecordLeaderDirWriter dirWriter(field0001len +1); 
    dirWriter.addField("DSPM", cellS57->getDspm().getIso8211Length());    
    bytesWritten += dirWriter.write(cellDev);

    //****** write the 0001 field ******
    bytesWritten += ISO8211::write0001fieldBIN(cellDev, currentIso8211rcid, field0001len +1);
    //****** write the DSPM Field (the only field in this record -except 0001) ******
    bytesWritten += writeDSPMField();

    return bytesWritten;
}

//*****************************************************************************
/// Write all Elements common to all SpatialS57-subclasses
/*!
* rem: a Subclass(edge) may have VRPT Records, but they are not stored for performance reasons,
*      but they have to be recreated when writing cell, so make an entry in the Directory, if needed
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeSpatial(const SpatialS57 & spatial, const char * coordTag, unsigned long coordSize, unsigned long vrptSize)
{
    unsigned long bytesWritten = 0;

    //** write Leader and Directory **
    const std::map < unsigned short, QString > & attibutes = spatial.getAttributes();
    std::map < unsigned short, QString >::const_iterator atIt;
    unsigned long allAttrLen = 0;
    for (atIt = attibutes.begin(); atIt != attibutes.end(); ++atIt)
    {
        allAttrLen += 2 +atIt->second.length() + 1; //AttrCode + Value + UT 
    }
    if (allAttrLen) ++allAttrLen; //final FT

    RecordLeaderDirWriter dirWriter(field0001len +1); 
    dirWriter.addField("VRID", spatial.getVRID().getIso8211Length());    
    if (allAttrLen) dirWriter.addField("ATTV", allAttrLen);
    if (vrptSize)   dirWriter.addField("VRPT", vrptSize);
    if (coordTag)   dirWriter.addField(coordTag, coordSize); //Spatial Field always at the End!
    bytesWritten += dirWriter.write(cellDev);

    //**** write the common fields ****
    bytesWritten += ISO8211::write0001fieldBIN(cellDev, currentIso8211rcid, field0001len +1); //"0001" Field is the first in any record!
    bytesWritten += writeVridField(spatial.getVRID());
    for (atIt = attibutes.begin(); atIt != attibutes.end(); ++atIt)
    {
        bytesWritten += ISO8211::writeUShort(cellDev, atIt->first);

        if (!atIt->second.isEmpty())
        {
            ulong attrBytes = cellDev->write(atIt->second.toLatin1(), atIt->second.length());
            if (attrBytes != atIt->second.length())
            {
                throw QString("ERROR: Spatial Attribute not written completely: only %1 of %2 bytes").arg(attrBytes).arg(atIt->second.length());
            }
            bytesWritten += attrBytes;
        }
        if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    }
    if (!attibutes.empty())
    {
        if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
    }
#ifdef _DEBUG
(unsigned int)spatial.recLenWrittn = dirWriter.recLenWrittn;
#endif
    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeNode(const NodeS57 & node)
{
    unsigned long bytesWritten = 0;

    bytesWritten += writeSpatial(node,"SG2D", 9);

    //** write the Spatial **
    bytesWritten += ISO8211::writeLong(cellDev, node.getLat() * cellS57->getDspm().getCOMF());     
    bytesWritten += ISO8211::writeLong(cellDev, node.getLon() * cellS57->getDspm().getCOMF());     
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
#ifdef _DEBUG
    if (bytesWritten != node.recLenWrittn) {
        printf("DEBUG: node: realBytesWritten = %d but recLenWritten = %d \n", bytesWritten, node.recLenWrittn);
    }
#endif
    return bytesWritten;
}

//*****************************************************************************
/// Write a 3D Field (only used for Soundings)
/*!
* In strange cases, there might be soundings without coordinates!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeSounding(const SoundgS57 & sndg)
{
    unsigned long bytesWritten = 0;
    const std::vector < double > & sg3dVec = sndg.getSG3Dvec(); 

    if (sg3dVec.empty())
    {
        bytesWritten += writeSpatial(sndg, 0, 0);
#ifdef _DEBUG
        printf("DEBUG: sounding without soundings!!\n");
#endif
    }
    else
    {
        int vertexSpace = sg3dVec.size() * 4 +1; //all triples  + FT
        bytesWritten += writeSpatial(sndg, "SG3D", vertexSpace);
    }

    // lat/lon/depth (Y,X,Z) coordiates, but already transformed by to double with  COMF and SOMF
    for(uint vertexI = 0; vertexI < sg3dVec.size(); vertexI += 3)
    {
        bytesWritten += ISO8211::writeLong(cellDev, sg3dVec[vertexI +0] * cellS57->getDspm().getCOMF()); //Lat     
        bytesWritten += ISO8211::writeLong(cellDev, sg3dVec[vertexI +1] * cellS57->getDspm().getCOMF()); //Lon
        bytesWritten += ISO8211::writeLong(cellDev, sg3dVec[vertexI +2] * cellS57->getDspm().getSOMF()); //Depth    
    }
    if (!sg3dVec.empty())
    {
        if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
    }
#ifdef _DEBUG
    if (bytesWritten != sndg.recLenWrittn) {
        printf("DEBUG: sndg: realBytesWritten = %d but recLenWritten = %d \n", bytesWritten, sndg.recLenWrittn);
    }
#endif
    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeEdge(const EdgeS57 & edge)
{
    unsigned long bytesWritten = 0;
    //** rem: the bounding Node Handles must be put into two VRPT fields ** 
    FieldVRPT startVrpt, endVrpt;
    makeVRPT4BoundingNode(startVrpt, edge.getStartNodeRecId(), true);
    makeVRPT4BoundingNode(endVrpt,   edge.getEndNodeRecId(),   false);

    //** Write Leader/Dir/Attributes **
    int vertexSpace = edge.getSG2Dvec().size() * 4 ; //All pairs 
    if (vertexSpace) ++vertexSpace; //1byte for FT, but only when there are vertices!
    if (edge.getSG2Dvec().size() == 0) //sometimes, edge has start/end-node, only!
    {
        bytesWritten += writeSpatial(edge, 0, 0, startVrpt.getIso8211Length() +  endVrpt.getIso8211Length() +1);
    }
    else bytesWritten += writeSpatial(edge, "SG2D", vertexSpace, startVrpt.getIso8211Length() +  endVrpt.getIso8211Length() +1);                     

    //** Write the Start/End-Node Handles **
    bytesWritten += writeVrptField(startVrpt);
    bytesWritten += writeVrptField(endVrpt);
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    //** lat/lon (Y,X) coordiates, but already transformed by to double with COMF **
    //** rem: SG2D vector may be empty!
    const std::vector < double > & sg2dVec = edge.getSG2Dvec(); 
    for(uint vertexI = 0; vertexI < sg2dVec.size(); vertexI += 2)
    {
        bytesWritten += ISO8211::writeLong(cellDev, sg2dVec[vertexI +0] * cellS57->getDspm().getCOMF()); //Lat     
        bytesWritten += ISO8211::writeLong(cellDev, sg2dVec[vertexI +1] * cellS57->getDspm().getCOMF()); //Lon
    }
    if (!sg2dVec.empty())
    {
        if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
    }
#ifdef _DEBUG
    if (bytesWritten != edge.recLenWrittn) {
        printf("DEBUG: edge: realBytesWritten = %d but recLenWritten = %d \n", bytesWritten, edge.recLenWrittn);
    }
#endif
    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
void CellWriter8211Dirty::makeVRPT4BoundingNode(FieldVRPT & vrpt, unsigned long recId, bool startNode) const
{
    vrpt.setOtherRCNM(ISO8211::RCNM_VC);
    vrpt.setOtherRCID(recId);
    vrpt.setORNT(255);
    vrpt.setUSAG(255);
    vrpt.setTOPI(startNode? 1 : 2);
    vrpt.setMASK(255);
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeVridField(const FieldVRID & vrid)
{
    unsigned long bytesWritten = 0;

    bytesWritten += ISO8211::writeUChar (cellDev, vrid.getRCNM()); 
    bytesWritten += ISO8211::writeULong (cellDev, vrid.getRCID());
    bytesWritten += ISO8211::writeUShort(cellDev, vrid.getRVER());
    bytesWritten += ISO8211::writeUChar (cellDev, vrid.getRUIN()); 
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeVrpcField(const FieldVRPC & vrpc)
{
    unsigned long bytesWritten = 0;

    bytesWritten += ISO8211::writeUChar (cellDev, vrpc.getVPUI()); 
    bytesWritten += ISO8211::writeUShort(cellDev, vrpc.getVPIX());
    bytesWritten += ISO8211::writeUShort(cellDev, vrpc.getNVPT());
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeVrptField(const FieldVRPT & vrpt)
{
    unsigned long bytesWritten = 0;

    bytesWritten += ISO8211::writeUChar (cellDev, vrpt.getOtherRCNM()); 
    bytesWritten += ISO8211::writeULong (cellDev, vrpt.getOtherRCID());
    bytesWritten += ISO8211::writeUChar (cellDev, vrpt.getORNT()); 
    bytesWritten += ISO8211::writeUChar (cellDev, vrpt.getUSAG()); 
    bytesWritten += ISO8211::writeUChar (cellDev, vrpt.getTOPI()); 
    bytesWritten += ISO8211::writeUChar (cellDev, vrpt.getMASK()); 
    //Hint: dont write final FT yet - this is a vector field !
    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeSGCCField(const FieldSGCC & sgcc)
{
    unsigned long bytesWritten = 0;

    bytesWritten += ISO8211::writeUChar (cellDev, sgcc.getCCUI()); 
    bytesWritten += ISO8211::writeUShort(cellDev, sgcc.getCCIX());
    bytesWritten += ISO8211::writeUShort(cellDev, sgcc.getCCNC());
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

//*****************************************************************************
/// write complete Feature to file
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeFeature(const FeatureS57 & feat)
{
    unsigned long bytesWritten = 0;

    //****** write leader and directory ******
    //** some Attributes are repeaded: calc size here:
    unsigned long ATTFsize =0, NATFsize =0, FFPTsize =0, FSPTsize =0;

    //**** Attributes: normal and national attributes are different! ****
    const std::map<unsigned short, FieldAttr> & attrVec = feat.getAttribs();
    std::map<unsigned short, FieldAttr>::const_iterator attrIt;
    //** 1st: all normal attributes **
    for (attrIt = attrVec.begin(); attrIt != attrVec.end(); ++attrIt)
    {
        if (!attrIt->second.getNat())ATTFsize += attrIt->second.getIso8211Length(cellS57->getDssi().getAALL() == 2);
    }
    if (ATTFsize) ++ATTFsize; //Final FT
    //**2nd: all natinal attributes **
    for (attrIt = attrVec.begin(); attrIt != attrVec.end(); ++attrIt)
    {
        if (attrIt->second.getNat()) NATFsize += attrIt->second.getIso8211Length(cellS57->getDssi().getNALL() == 2);
    }
    if (NATFsize) ++NATFsize; //Final FT
    const std::vector < FieldFFPT > & FFPTvec = feat.getFfptVec();
    std::vector < FieldFFPT >::const_iterator ffptIt;
    for (ffptIt = FFPTvec.begin(); ffptIt != FFPTvec.end(); ++ffptIt)
    {
        FFPTsize += ffptIt->getIso8211Length();
    }
    if (FFPTsize) ++FFPTsize; //Final FT
    const std::vector < FieldFSPT > & FSPTvec = feat.getFsptVec();
    std::vector < FieldFSPT >::const_iterator fsptIt;
    for (fsptIt = FSPTvec.begin(); fsptIt != FSPTvec.end(); ++fsptIt)
    {
        FSPTsize += fsptIt->getIso8211Length();
    }
    if (FSPTsize) ++FSPTsize; //Final FT
    RecordLeaderDirWriter dirWriter(field0001len +1); 
    dirWriter.addField("FRID", feat.getFRID().getIso8211Length());    
    dirWriter.addField("FOID", feat.getFOID().getIso8211Length());  
    if (ATTFsize) dirWriter.addField("ATTF", ATTFsize);
    if (NATFsize) dirWriter.addField("NATF", NATFsize);
    if (cellS57->isUpdateCell())
    {
        //dirWriter.addField("FFPC", feat.getFFPC().getIso8211Length()); 
    }
    if (FFPTsize) dirWriter.addField("FFPT", FFPTsize);
    if (cellS57->isUpdateCell()) 
    {
        //dirWriter.addField("FSPC", feat.getFSPC().getIso8211Length()); 
    }
    if (FSPTsize) dirWriter.addField("FSPT", FSPTsize);
    bytesWritten += dirWriter.write(cellDev);

    //****** write the Fields ******
    bytesWritten += ISO8211::write0001fieldBIN(cellDev, currentIso8211rcid, field0001len +1);  //"0001" field: unimportant, but must be first in any Record!!
    bytesWritten += writeFridField(feat.getFRID());
    bytesWritten += writeFoidField(feat.getFOID());

    //**** Attributes: normal and national attributes are different! ****
    //** 1st: all normal attributes **
    for (attrIt = attrVec.begin(); attrIt != attrVec.end(); ++attrIt)
    {
        if (attrIt->second.getNat() == false) bytesWritten += writeAttributeField(attrIt->second);
    }
    if (ATTFsize) 
    {
        if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
    }
    //**2nd: all natinal attributes **
    for (attrIt = attrVec.begin(); attrIt != attrVec.end(); ++attrIt)
    {
        if (attrIt->second.getNat() == true) bytesWritten += writeAttributeField(attrIt->second);
    }
    if (NATFsize)
    {
        if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
    }
    if (cellS57->isUpdateCell())
    {
        //bytesWritten += writeFfpcField(feat.getFFPC()); 
    }
    for (ffptIt = FFPTvec.begin(); ffptIt != FFPTvec.end(); ++ffptIt)
    {
        bytesWritten += writeFfptField(*ffptIt);
    }
    if (!FFPTvec.empty())
    {
        if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
    }
    if (cellS57->isUpdateCell()) 
    {
        //bytesWritten += writeFspcField(feat.getFSPC());
    }
    for (fsptIt = FSPTvec.begin(); fsptIt != FSPTvec.end(); ++fsptIt)
    {
        bytesWritten += writeFsptField(*fsptIt);
    }
    if (!FSPTvec.empty())
    {
        if (cellDev->putChar(ISO8211::FT)) bytesWritten++;
    }

#ifdef _DEBUG
    if (bytesWritten != dirWriter.recLenWrittn) {
        printf("DEBUG: realBytesWritten = %d but recLenWritten = %d \n", bytesWritten, dirWriter.recLenWrittn);
    }
#endif
    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeDSIDField()
{
    unsigned long bytesWritten = 0;

    const FieldDSID & dsid = cellS57->getDsid();

    bytesWritten += ISO8211::writeUChar(cellDev, dsid.getRCNM()); 
    bytesWritten += ISO8211::writeULong(cellDev, dsid.getRCID());
    bytesWritten += ISO8211::writeUChar(cellDev, dsid.getEXPP()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dsid.getINTU()); 
    bytesWritten += cellDev->write(dsid.getDSNM().toLatin1());  //rem Dataset name is never unicode!
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    bytesWritten += cellDev->write(ISO8211::number2String(dsid.getEDTN(), Digits4EditionNo).toLatin1());  //Edition Number
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    bytesWritten += cellDev->write(ISO8211::number2String(dsid.getUPDN(), Digits4UpdateNo).toLatin1());  //Edition Number
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    bytesWritten += cellDev->write(dsid.getUADT(), 8);   //Update Date - MUST be 8 Bytes!
    bytesWritten += cellDev->write(dsid.getISDT(), 8);   //Issue Date - MUST be 8 Bytes!
    bytesWritten += cellDev->write(dsid.getSTED(), 4);   //Edition of S57 Standard, "03.1" - MUST be 4 Bytes!
    bytesWritten += ISO8211::writeUChar(cellDev, dsid.getPRSP()); 
    bytesWritten += cellDev->write(dsid.getPSDN().toLatin1());  //rem: Product Specification description is never unicode!
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    bytesWritten += cellDev->write(dsid.getPRED().toLatin1());  //rem: Product Specification edition is never unicode!
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    bytesWritten += ISO8211::writeUChar(cellDev, dsid.getPROF()); 
    bytesWritten += ISO8211::writeUShort(cellDev, dsid.getAGEN()); 
    bytesWritten += cellDev->write(dsid.getCOMT().toLatin1());  //rem: Comment is never unicode!
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    //** all over: write the Field Termiator **
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeDSSIField()
{
    unsigned long bytesWritten = 0;
    
    const FieldDSSI & dssi = cellS57->getDssi();

    bytesWritten += ISO8211::writeUChar(cellDev, dssi.getDSTR()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dssi.getAALL()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dssi.getNALL()); 
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOMR());
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOCR());
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOGR());
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOLR());
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOIN());
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOCN());
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOED());
    bytesWritten += ISO8211::writeULong(cellDev, dssi.getNOFA());
    //** all over: write the Field Termiator **
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
unsigned long CellWriter8211Dirty::writeDSPMField()
{
    unsigned long bytesWritten = 0;

    const FieldDSPM & dspm = cellS57->getDspm();

    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getRCNM()); 
    bytesWritten += ISO8211::writeULong(cellDev, dspm.getRCID());
    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getHDAT()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getVDAT()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getSDAT()); 
    bytesWritten += ISO8211::writeULong(cellDev, dspm.getCSCL());
    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getDUNI()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getHUNI()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getPUNI()); 
    bytesWritten += ISO8211::writeUChar(cellDev, dspm.getCOUN()); 
    bytesWritten += ISO8211::writeULong(cellDev, dspm.getCOMF());
    bytesWritten += ISO8211::writeULong(cellDev, dspm.getSOMF());
    bytesWritten += cellDev->write(dspm.getCOMT().toLatin1(), dspm.getCOMT().length());  //rem: Comment is never unicode!
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    //** all over: write the Field Termiator **
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

//** feature Record Fields **

unsigned long CellWriter8211Dirty::writeFridField(const FieldFRID & frid)
{
    unsigned long bytesWritten = 0;
    bytesWritten += ISO8211::writeUChar(cellDev, frid.getRCNM()); 
    bytesWritten += ISO8211::writeULong(cellDev, frid.getRCID());
    bytesWritten += ISO8211::writeUChar(cellDev, frid.getPRIM()); 
    bytesWritten += ISO8211::writeUChar(cellDev, frid.getGRUP()); 
    bytesWritten += ISO8211::writeUShort(cellDev, frid.getOBJL());
    bytesWritten += ISO8211::writeUShort(cellDev, frid.getRVER());
    bytesWritten += ISO8211::writeUChar(cellDev, frid.getRUIN()); 
    //** all over: write the Field Termiator **
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}
unsigned long CellWriter8211Dirty::writeFoidField(const FieldFOID & foid)
{
    unsigned long bytesWritten = 0;
    bytesWritten += ISO8211::writeULongLong(cellDev, foid.getLongNAMe());
    //** all over: write the Field Termiator **
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

unsigned long CellWriter8211Dirty::writeAttributeField(const FieldAttr & attrib)
{
    unsigned long bytesWritten = 0;
    bytesWritten += ISO8211::writeUShort(cellDev, attrib.getATTL());

    //** attibute values may be 8 or 16 bit encoded ! -> same for UT **
    bool to16bitUnicode = false;
    if (attrib.getNat())
    {
        if (cellS57->dssi.getNALL() == 2) to16bitUnicode = true;
    }
    else
    {
        if (cellS57->dssi.getAALL() == 2) to16bitUnicode = true;
    }
    if (to16bitUnicode)
    {
        ulong attrWriteLen = cellDev->write(reinterpret_cast<const char *>(attrib.getValue().utf16(), attrib.getValue().length() * 2));
        if (attrWriteLen != attrib.getValue().length() *2) throw QString("ERROR: Unicode Feature Attribute (class %1) not written correctly: %2").arg(attrib.getATTL()).arg(attrib.getValue());
        bytesWritten += cellDev->write(reinterpret_cast<const char *>(&ISO8211::UTL2), 2);
    }
    else
    {
        ulong attrWriteLen = cellDev->write(attrib.getValue().toLatin1(), attrib.getValue().length());
        if (attrWriteLen != attrib.getValue().length()) throw QString("ERROR: Feature Attribute  (class %1) not written correctly: %2").arg(attrib.getATTL()).arg(attrib.getValue());
        bytesWritten += attrWriteLen;
        if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    }

    return bytesWritten;
}

unsigned long CellWriter8211Dirty::writeFfpcField(const FieldFFPC & ffpc)
{
    unsigned long bytesWritten = 0;

    bytesWritten += ISO8211::writeUChar(cellDev, ffpc.getFFUI()); 
    bytesWritten += ISO8211::writeUShort(cellDev, ffpc.getFFIX());
    bytesWritten += ISO8211::writeUShort(cellDev, ffpc.getNFPT());
    //** all over: write the Field Termiator **
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

unsigned long CellWriter8211Dirty::writeFfptField(const FieldFFPT & ffpt)
{
    unsigned long bytesWritten = 0;
    bytesWritten += ISO8211::writeULongLong(cellDev, ffpt.getLNAM());
    bytesWritten += ISO8211::writeUChar(cellDev, ffpt.getRIND()); 
    bytesWritten += cellDev->write(ffpt.getCOMT().toLatin1());  //rem: Product Specification edition is never unicode!
    if (cellDev->putChar(ISO8211::UT)) bytesWritten++;
    //Hint: dont write the Field Termiator yet - this is a vector field!

    return bytesWritten;
}

unsigned long CellWriter8211Dirty::writeFspcField(const FieldFSPC & fspc)
{
    unsigned long bytesWritten = 0;
    bytesWritten += ISO8211::writeUChar(cellDev, fspc.getFSUI()); 
    bytesWritten += ISO8211::writeUShort(cellDev, fspc.getFSIX());
    bytesWritten += ISO8211::writeUShort(cellDev, fspc.getNSPT());
    //** all over: write the Field Termiator **
    if (cellDev->putChar(ISO8211::FT)) bytesWritten++;

    return bytesWritten;
}

unsigned long CellWriter8211Dirty::writeFsptField(const FieldFSPT & fspt)
{
    unsigned long bytesWritten = 0;

    //** Rem: the 40 bit pointer to spatial is stored as 2 values: char and long **
    bytesWritten += ISO8211::writeUChar(cellDev, fspt.getOtherRCNM());
    bytesWritten += ISO8211::writeULong(cellDev, fspt.getOtherRCID());

    bytesWritten += ISO8211::writeUChar(cellDev, fspt.getORNT()); 
    bytesWritten += ISO8211::writeUChar(cellDev, fspt.getUSAG()); 
    bytesWritten += ISO8211::writeUChar(cellDev, fspt.getMASK()); 
    //Hint: dont write the Field Termiator yet - this is a vector field!

    return bytesWritten;
}

//*****************************************************************************
//**********************  LeaderAndDirectory Writer  **************************
//*****************************************************************************

//*****************************************************************************
/// Write Leader and Directory of a S-57 Iso8211 Recored
/*! 
****************************************************************************** */
unsigned long RecordLeaderDirWriter::write(QIODevice * device) const
{
    QTextStream iso8211strm(device);
    iso8211strm.setFieldAlignment(QTextStream::AlignRight);
    iso8211strm.setPadChar('0');

    std::vector< ISO8211::DirEntry >::const_iterator eIt;

    //**** calculate values for sizeofFieldLenField and sizeofFieldPosField ****
    //**** and correct initial values if too short ****
    int realSizeFldLenFld, realSizeFldPosFld;
    long maxFieldLen = 0;
    for(eIt = dirEntries.begin(); eIt != dirEntries.end(); ++eIt)
    {
        if ((*eIt).fieldLen > maxFieldLen) maxFieldLen = (*eIt).fieldLen;
    }
    realSizeFldLenFld = ISO8211::decDigits4Number(maxFieldLen);
    if (sizeFldLenFld > realSizeFldLenFld) realSizeFldLenFld = sizeFldLenFld;

    realSizeFldPosFld = ISO8211::decDigits4Number(dirEntries[dirEntries.size() -1].fieldPos); //last entry holds biggest pos. ,of course
    if (sizeFldPosFld > realSizeFldPosFld) realSizeFldPosFld = sizeFldPosFld;
    
    //** write the Recored Leader **
    unsigned int recFieldStart = 24 + dirEntries.size() * (4 + realSizeFldLenFld + realSizeFldPosFld) +1; //length: leader(24 Bytes) + all Dir Entries + FT **
    unsigned int reclen = recFieldStart + dirEntries[dirEntries.size() -1].fieldPos + dirEntries[dirEntries.size() -1].fieldLen;

    ISO8211::writeRecLeaderS57(iso8211strm, reclen, recFieldStart, realSizeFldLenFld, realSizeFldPosFld);

    //** write the directory entries **
    for(eIt = dirEntries.begin(); eIt != dirEntries.end(); ++eIt)
    {
        iso8211strm.setFieldWidth(4);
        iso8211strm <<  QString::fromAscii(eIt->tag, 4);
        iso8211strm.setFieldWidth(realSizeFldLenFld);
        iso8211strm << eIt->fieldLen;
        iso8211strm.setFieldWidth(realSizeFldPosFld);
        iso8211strm << eIt->fieldPos;
    }
    iso8211strm.setFieldWidth(0);
    iso8211strm << ISO8211::FT; 

#ifdef _DEBUG
     (unsigned int)recLenWrittn = reclen;
#endif
    return recFieldStart; //rem: length: leader(24 Bytes) + all Dir Entries + FT = start of data in recored
}

//*****************************************************************************
/// Add one entry to the end of the directory
/*!
* total0001Fieldlen = length of 0001 Field, INCLUDING FT !!
****************************************************************************** */
void RecordLeaderDirWriter::addField(const char * tag, int lenght)
{
    //** add entry for "0001" field, if not yet done - always the FIRST entry! **
    if (dirEntries.empty()) 
    {
        ISO8211::DirEntry firstEntry;
        strncpy(firstEntry.tag, "0001", 4);
        firstEntry.fieldLen = total0001Fieldlen; 
        dirEntries.push_back(firstEntry);
    }
    //** add new entry and calcualte position **
    const ISO8211::DirEntry & lastEntry = dirEntries[dirEntries.size() -1];
    ISO8211::DirEntry entry;
    strncpy(entry.tag, tag, 4);
    entry.fieldLen = lenght;
    entry.fieldPos = lastEntry.fieldPos + lastEntry.fieldLen;
    
    dirEntries.push_back(entry);
}