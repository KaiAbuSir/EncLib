//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include <QtCore/QFile>

#include "cell_parser_iso8211dirty.h"
#include "cell_s57.h"
#include "cell_records.h"
#include "iso8211_simple.h"

using namespace Enc;

//*****************************************************************************
/// Konstructor does some inits, only
//*****************************************************************************
CellParser8211Dirty::CellParser8211Dirty(unsigned long parseOptions, double factorXY, double factorZ) 
                   : CellParser8211(parseOptions), fileBuffer(0), fileBufSize(0), fileSize(0), facXY(factorXY), facZ(factorZ), AALL(0), NALL(0),
                     unusedVRPT(0), unusedSpatTag(0), unusedFeatTag(0)
{
}

CellParser8211Dirty::~CellParser8211Dirty()
{
    delete [] fileBuffer;
}

//*****************************************************************************
/// Prepare Parsing: Load the cell into memory, save DDR if needed
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseInit(QString cellName, CellS57_Header * cell, uint bytes2read)
{
    //**** clear old data ****
    cell->clear();

    unusedVRPT = 0; 
    unusedFeatTag = 0;
    unusedSpatTag = 0;

    //**** load iso8211 file into memory ****
    QFile file8211Qf(cellName);
    if (!file8211Qf.open(QIODevice::ReadOnly))
    {
        QString("Error: cannot open file: %1").arg(cellName);
    }

    //** load file into memory, reallocate if needed **
    fileSize = file8211Qf.size();
    //** in case we do not want to read the beginning of the file: ==
    if (bytes2read > 0 && fileSize > bytes2read) 
    {
        fileSize = bytes2read;
    }
    if (fileSize > fileBufSize)
    {
        fileBufSize = 0;
        delete [] fileBuffer ;
        fileBuffer = new char[fileSize];
        fileBufSize = fileSize;
    }
    memset(fileBuffer, 0, fileBufSize);  //Not needed (but easier debugging)
    qint64 bypesRead = file8211Qf.read(fileBuffer, fileSize);
    file8211Qf.close();

    if (parseOpts & StoreDDR)
    {
        unsigned int DDRlen;
        ISO8211::checkDDRleader(fileBuffer, fileSize, &DDRlen);
        if (DDRlen > fileSize) throw QString("ERROR: Cannot copy DDR - only %1 of %2 bytes read").arg(DDRlen).arg(fileSize);
        cell->putDDR(fileBuffer, DDRlen);
    }
}

//*****************************************************************************
/// Parse the complete cell
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseS57CellIntern(CellS57_Header * cell)
{

    //**** Parse S-57 cell ****
    Iso8211recordIterator rcIt;
    rcIt.init(fileBuffer, fileSize);
    
    //**** Iterate over All Records ****
    bool vectorRecsDone = false; //little speedup (Rem: VectorRecords are situated before FeatureRecords)
    int recInd = 0;
    for (; rcIt.current(); rcIt.next(), ++recInd)
    {
        int currRecLen = 0;
        const char * currRec = rcIt.current(&currRecLen);

        //**** Iterate over all Record-Fields ****
        Iso8211fieldIterator fieldIt(currRec);
        unsigned int fieldInd = 0;
        for (; fieldInd < fieldIt.fieldCount(); ++fieldInd)
        {
            int fieldSize;
            const char * tag = 0;
            const char * field = fieldIt.at(fieldInd, &fieldSize, tag);
            //** first field is always the 0001-field -> it is not of interest, only used interally for 8211
            if (fieldInd == 0) //better but slower: (strncmp(tag, "0001", 4) == 0) 
            {
                continue; 
            }
            //D.S. Gen. Info. Rec. is always at first, no need to test tags
            else if (recInd == 0)  
            {
                if      (strncmp(tag, "DSID", 4) == 0) parseDSIDField(field, fieldSize, cell->getDsid());
                else if (strncmp(tag, "DSSI", 4) == 0) parseDSSIField(field, fieldSize, cell->getDssi());
                else
                {
                    //invalid field -> ignore it ?!
                }
            }
            //**** Parse Vector Records ****
            else if (!vectorRecsDone && strncmp(tag, "VRID", 4) == 0)
            {
                if ((parseOpts & PrsVecRec) == 0 &&  (parseOpts & PrsFeatRec) == 0) break; // in case only CellHeader is needed
                if ((parseOpts & PrsVecRec) == 0) continue; //in case Geometry is not needed

                parseVectorRecord(fieldIt);
            }
            //**** Parse Feature Record ****
            else if (strncmp(tag, "FRID", 4) == 0)
            {
                vectorRecsDone = true;
                if ((parseOpts & PrsFeatRec) == 0) break; //in case Features are not needed 

                parseFeatureRecord(fieldIt);
            }
            //**** Parse GeoRefRecord - test at last: only 1 or 0 in a File ****
            else if (strncmp(tag, "DSPM", 4) == 0)
            {
                parseDSPMField(field, fieldSize, cell->getDspm());
            }
        }
#ifdef _DEBUG
        if(cell->getClass() == CellS57_Header::BaseCellClass && fieldIt.fieldCount() < 3) printf("DEBUG: Record %d has only %d fields!\n", recInd, fieldIt.fieldCount());
#endif
    }
#ifdef _DEBUG
    printf("DEBUG: %d Records parsed!\n", recInd );
#endif
}

//*****************************************************************************
/// Parse the DSID field of the Cell Header (first field)
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseDSIDField(const char * fieldPtr, int fieldLen, FieldDSID & dsid)
{
    const char * endPtr = fieldPtr + fieldLen;
    int offset = 0;

    //**** 1. Record Name, always 10 (bin) (accii "DS") - unimportant****
    dsid.setRCNM(ISO8211::rcnm2ascii(fieldPtr));     
    fieldPtr += 1;

    //**** 2. Record-id - unimportant ****
    dsid.setRCID(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;

    //**** 3. Exchange purpose - 'N' (new) or 'R' (revision)
    dsid.setEXPP(*(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;

    //**** 4: usage ****
    dsid.setINTU(*(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;

    //**** 5. Data Set Name ****
    offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    dsid.setDSNM(QString::fromLatin1(fieldPtr, offset));  //
    fieldPtr += offset +1;

    //**** 6. Edition Number ****
    offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    dsid.setEDTN(ISO8211::str2uInt(fieldPtr, offset));
    fieldPtr += offset +1;

    //**** 7. Update Number ****
    offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    dsid.setUPDN(ISO8211::str2uInt(fieldPtr, offset));  
    fieldPtr += offset +1;

    //**** 8. Update application Date ****
    dsid.setUADT(fieldPtr);  
    fieldPtr += 8;

    //**** 9. Issue Date ****
    dsid.setISDT(fieldPtr);
    fieldPtr += 8;

    //**** 10. Edition Number S-57 always "03.1" - unimportant ****
    dsid.setSTED(fieldPtr);  
    fieldPtr += 4;

    //**** 11. Product Specification - normally "ENC"  ****
    dsid.setPRSP(*(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;

    //**** 12. Product Specification description - unimportant ****
    offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    dsid.setPSDN(QString::fromLatin1(fieldPtr, offset)); 
    fieldPtr += offset +1;

    //**** 13. Product Specification edition number - unimportant ****
    offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    dsid.setPRED(QString::fromLatin1(fieldPtr, offset)); 
    fieldPtr += offset +1;

    //**** 14. Application profile identification - "EN" "ER" or "DD"  ****
    dsid.setPROF(*(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;

    //**** 15. Producing agency ****
    dsid.setAGEN(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;

    //**** 16: Comment ****
    offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    dsid.setCOMT(QString::fromLatin1(fieldPtr, offset));
}

//*****************************************************************************
/// Parse the DSSI field of the Cell Header (second field)
/*!
* 
****************************************************************************** */
void CellParser8211Dirty::parseDSSIField(const char * fieldPtr, int fieldLen, FieldDSSI & dssi)
{
    //Topology: should be 3(PG = Planar Graph) or 0 (NO = Not Relevant)
    dssi.setDSTR(*(reinterpret_cast<const unsigned char *>(fieldPtr))); 
    fieldPtr += 1;

    //Lexical level of ATTF fields
    dssi.setAALL(*(reinterpret_cast<const unsigned char *>(fieldPtr))); 
    fieldPtr += 1;

    //Lexical level of NATF fields
    dssi.setNALL(*(reinterpret_cast<const unsigned char *>(fieldPtr)));  
    fieldPtr += 1;

    //**** record-counter subfields: ****
    dssi.setNOMR(ISO8211::bytes2ulong(fieldPtr)); //meta records
    fieldPtr += 4;
    dssi.setNOCR(ISO8211::bytes2ulong(fieldPtr)); //cartographic records - sould be 0 !!!
    fieldPtr += 4;
    dssi.setNOGR(ISO8211::bytes2ulong(fieldPtr)); //number of geo records
    fieldPtr += 4;
    dssi.setNOLR(ISO8211::bytes2ulong(fieldPtr)); //number of collection records
    fieldPtr += 4;
    dssi.setNOIN(ISO8211::bytes2ulong(fieldPtr)); //number of isolated nodes
    fieldPtr += 4;
    dssi.setNOCN(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;
    dssi.setNOED(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;
    dssi.setNOFA(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;

    AALL = dssi.getAALL();
    NALL = dssi.getNALL();
}

//*****************************************************************************
/// Parse the DSPM Field of the Cell Header
/*!
* Rem: Only in "EN"-Cells
****************************************************************************** */
void CellParser8211Dirty::parseDSPMField(const char * fieldPtr, int fieldLen, FieldDSPM & dspm)
{
    const char * endPtr = fieldPtr + fieldLen;
    
    dspm.setRCNM(ISO8211::rcnm2ascii(fieldPtr)); //Record Name - always "DP" (20) 
    fieldPtr += 1;

    dspm.setRCID(ISO8211::bytes2ulong(fieldPtr));    //Record Identification Number: Ranges: 1 to 2^32-2
    fieldPtr += 4;

    dspm.setHDAT(*(reinterpret_cast<const unsigned char *>(fieldPtr)));    //Horizontal Datum - in S-57 always WGS84
    fieldPtr += 1;
    dspm.setVDAT(*(reinterpret_cast<const unsigned char *>(fieldPtr)));    //Vertical Datum
    fieldPtr += 1;
    dspm.setSDAT(*(reinterpret_cast<const unsigned char *>(fieldPtr)));    //Sounding Datum
    fieldPtr += 1;
    dspm.setCSCL(ISO8211::bytes2ulong(fieldPtr));    //Compilation Scale
    fieldPtr += 4;
    dspm.setDUNI(*(reinterpret_cast<const unsigned char *>(fieldPtr)));;    
    fieldPtr += 1;
    dspm.setHUNI(*(reinterpret_cast<const unsigned char *>(fieldPtr)));;
    fieldPtr += 1;
    dspm.setPUNI(*(reinterpret_cast<const unsigned char *>(fieldPtr)));;
    fieldPtr += 1;
    dspm.setCOUN(*(reinterpret_cast<const unsigned char *>(fieldPtr))); //Coordiante Units - always 1 ="LL"(lat lon) for S-57
    fieldPtr += 1;
    dspm.setCOMF(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;
    dspm.setSOMF(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;
   
    //****  Comment ****
    int offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    dspm.setCOMT(QString::fromLatin1(fieldPtr, offset));

    //**** cast SOMF and COMF to double - needed later when parsing coordinates ****
    facXY = dspm.getCOMF();
    facZ  = dspm.getSOMF();
}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
 void CellParser8211Dirty::parseFeatureRecord(Iso8211fieldIterator & fieldIt, FeatureS57 & feat)
 {
    unsigned int fieldInd = 0;
    for (; fieldInd < fieldIt.fieldCount(); ++fieldInd)
    {
        int fieldSize;
        const char * tag = 0;
        const char * fieldPtr = fieldIt.at(fieldInd, &fieldSize, tag);

        if (!parseFeatureField(fieldSize, tag, fieldPtr, feat))
        {
            

        
        }
    }
 }

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseFeatureUpdtRecord(Iso8211fieldIterator & fieldIt, FeatureS57_Updt & feat)
{
    unsigned int fieldInd = 0;
    for (; fieldInd < fieldIt.fieldCount(); ++fieldInd)
    {
        int fieldSize;
        const char * tag = 0;
        const char * fieldPtr = fieldIt.at(fieldInd, &fieldSize, tag);
        if (!parseFeatureField(fieldSize, tag, fieldPtr, feat))
        {
            if (strncmp(tag, "FFPC", 4) == 0)
            {
                parseFFPCField(fieldPtr, fieldSize, feat.getFFPC());
            }
            else if (strncmp(tag, "FSPC", 4) == 0)
            {
                parseFSPCField(fieldPtr, fieldSize, feat.getFSPC());
            }
            else
            {
                printf("\nWARNING: Unknown Update Feature Record Field Tag: %s", tag);
                ++unusedFeatTag;
            }
        }
    }   
}

//*****************************************************************************
/// Parse all Fields of a Feature Record of a Base ("EN") cell
/*!
* Returns true if all Records could be parsed
****************************************************************************** */
bool CellParser8211Dirty::parseFeatureField(int & fieldSize, const char * tag, const char * field, FeatureS57 & feat)
{
    //** attributes and national attributes are the most likley subfields **
    if      (strncmp(tag, "ATTF", 4) == 0)
    {
        while (fieldSize > 1)
        {
            long bytesParsed = parseATTF_NAFTField(field, fieldSize, AALL, false, feat);
            field += bytesParsed;
            fieldSize -= bytesParsed;
        }
    }
    else if (strncmp(tag, "NAFT", 4) == 0)
    {
        while (fieldSize > 1)
        {
            long bytesParsed = parseATTF_NAFTField(field, fieldSize, NALL, true, feat); 
            field += bytesParsed;
            fieldSize -= bytesParsed;
        }
    }
    else if (strncmp(tag, "FRID", 4) == 0) parseFRIDField(field, fieldSize, feat.getFRID());
    else if (!(parseOpts & FeatTextOnly))
    {
        //** parse the pointer Fields (not needed if only attributes are read) **
        if      (strncmp(tag, "FOID", 4) == 0) parseFOIDField(field, fieldSize, feat.getFOID());
        //only update: else if (strncmp(tag, "FFPC", 4) == 0) parseFFPCField(field, fieldSize, feat.getFFPC());
        else if (strncmp(tag, "FFPT", 4) == 0)
        {
            while (fieldSize > 1)
            {
                long bytesParsed = parseFFPTField(field, fieldSize, feat);
                field += bytesParsed;
                fieldSize -= bytesParsed;
            }
        }
        //else if (strncmp(tag, "FSPC", 4) == 0) parseFSPCField(field, fieldSize, feat.getFSPC());
        else if (strncmp(tag, "FSPT", 4) == 0)
        {
            while (fieldSize > 1)
            {
                long bytesParsed = parseFSPTField(field, fieldSize, feat);
                field += bytesParsed;
                fieldSize -= bytesParsed;
            }
        }
        //ISO8211 Record-id: is not really needed, only for iso8211 internally
        else if (strncmp(tag, "0001", 4) == 0) 
        {
            if (fieldSize>4)
            {
                feat.setISO8211recid(ISO8211::bytes2ulong(field));  //this is against the S-57 Standard!!!
            }
            else feat.setISO8211recid(ISO8211::bytes2ushort(field)); //normal case: recid is 2bytes, binary, unsigned
        }
        else
        {
            return false;
        }
    }
    return true;
}

//*****************************************************************************
/// 
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseFRIDField(const char * fieldPtr, int fieldLen, FieldFRID & frid)
{
    const char * endPtr = fieldPtr + fieldLen;

    //**** 1. Record Name, always 100 (bin) (accii "FE") - unimportant****
    frid.setRCNM(*(reinterpret_cast<const unsigned char *>(fieldPtr))); 
    fieldPtr += 1;

    //**** 2. Record-id - unimportant ****
    frid.setRCID(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;

    //**** 3. geo.Primitive: 1,2,3 or 255 P,L,N or N
    frid.setPRIM( *(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;

    //**** 4. Group: 1,2, or 255
    frid.setGRUP(*(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;

    //**** 5. Object label
    frid.setOBJL(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;

    //**** 6: Record Version Ser. Num. - unimportant ****
    frid.setRVER(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;

    //**** 7. Record Update Instruction = 1,2 or 3 = I,D or M
    frid.setRUIN(*(reinterpret_cast<const unsigned char *>(fieldPtr)));

    if (fieldPtr > endPtr) throw QString("ERROR: FRID Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseFOIDField(const char * fieldPtr, int fieldLen, FieldFOID & foid)
{
    const char * endPtr = fieldPtr + fieldLen;

    //Agency code 
    foid.setAGEN(ISO8211::bytes2ushort(fieldPtr)); 
    fieldPtr += 2;

    //1 to 2^32 -2
    foid.setFIDN(ISO8211::bytes2ulong(fieldPtr)); 
    fieldPtr += 4;

    //1 to 2^16 -2
    foid.setFIDS(ISO8211::bytes2ushort(fieldPtr));    
    fieldPtr += 2;

    if (fieldPtr > endPtr) throw QString("ERROR: FOID Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
/// Parse Attributes (LexLevel 0,1 or 2) Unicode, ASCII or Latin1
/*!
* in S-57 only NATF are allowed be LexLevel2, but it does not matter to allow it for ATTF, too
* Returns the number of bytes parsed
****************************************************************************** */
long CellParser8211Dirty::parseATTF_NAFTField(const char * fieldPtr, int fieldLen, int lexLevel, bool isNat, FeatureS57 & feat)
{
    long bytesParsed = 0;
    const char * endPtr = fieldPtr + fieldLen;
    FieldAttr attrib;
    attrib.setNat(isNat);

    //**** attribute code ****
    attrib.setATTL(ISO8211::bytes2ushort(fieldPtr));    
    fieldPtr += 2;
    bytesParsed += 2;

    int offset = 0;
    //**** Level 2 - assume UTF16 attribute (16bit) ****
    if (lexLevel >= 2)
    {
        offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UTL2, endPtr);
        attrib.setValue(ISO8211::parseLexLevel2(fieldPtr, offset));
        bytesParsed += 2 * offset +2;
    }
    //**** Level 0 or 1 - assume latin1 (althougt level0 is only ASCII)****
    else
    {
        offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
        attrib.setValue(QString::fromLatin1(fieldPtr, offset));
        bytesParsed += offset +1;
    }
    feat.getAttribs()[attrib.getATTL()] = attrib;
    
    return bytesParsed;
}

//*****************************************************************************
/// Control Field: FeatureRec to FeatureObj
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseFFPCField(const char * fieldPtr, int fieldLen, FieldFFPC & ffpc)
{
    const char * endPtr = fieldPtr + fieldLen;

    //1,2,3 = Insert, Delete Modify
    ffpc.setFFUI(*(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;
    ffpc.setFFIX(ISO8211::bytes2ushort(fieldPtr)); 
    fieldPtr += 2;
    ffpc.setNFPT(ISO8211::bytes2ushort(fieldPtr)); 
    fieldPtr += 2;
    if (fieldPtr > endPtr) throw QString("ERROR: FFPC Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
/// Relationship: FeatureRec to FeatureObj  
/*!
*
****************************************************************************** */
long CellParser8211Dirty::parseFFPTField(const char * fieldPtr, int fieldLen, FeatureS57 & feat)
{
    const char * endPtr = fieldPtr + fieldLen;

    FieldFFPT ffpt;

    //64 bit field: Actually not a long long ,but: Concatination:  AGEN + FIDN + FIDS of FOID-field
    //would make byteorder Problems: ffpt.LNAM = *(reinterpret_cast<const unsigned long long *>(fieldPtr)) ; 

    FieldFOID otherFOID;
    parseFOIDField(fieldPtr, 64, otherFOID);
    ffpt.setLNAM(otherFOID.getLongNAMe());

    fieldPtr += 8;
    ffpt.setRIND(*(reinterpret_cast<const unsigned char *>(fieldPtr))); //1,2 or 3 = Master, Slave or Peer (M,S,P)
    fieldPtr += 1;
    int offset = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    ffpt.setCOMT(QString::fromLatin1(fieldPtr, offset)); //comment
    fieldPtr += offset +1;
    if (fieldPtr > endPtr) throw QString("ERROR: FFPT Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);

    feat.getFfptVec().push_back(ffpt);
    return 8 + 1 + offset + 1;
}

//*****************************************************************************
///
/*!
*
****************************************************************************** */
void CellParser8211Dirty::parseFSPCField(const char * fieldPtr, int fieldLen, FieldFSPC & fspc)
{
    const char * endPtr = fieldPtr + fieldLen;

    fspc.setFSUI(*(reinterpret_cast<const unsigned long long *>(fieldPtr))); //1,2,3 = Insert, Delete Modify
    fieldPtr += 1;
    fspc.setFSIX(ISO8211::bytes2ushort(fieldPtr)); 
    fieldPtr += 2;
    fspc.setNSPT(ISO8211::bytes2ushort(fieldPtr)); 
    fieldPtr += 2;

    if (fieldPtr > endPtr) throw QString("ERROR: FFPT Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
/// Feature to Spatial Pointer
/*!
*
****************************************************************************** */
long CellParser8211Dirty::parseFSPTField(const char * fieldPtr, int fieldLen, FeatureS57 & feat)
{
    const char * endPtr = fieldPtr + fieldLen;
    FieldFSPT fspt;

    //rem: unsigned char NAME[5]; //Pointer to Spatial = RCNM + RCID (1+4Byte)
    fspt.setOtherRCNM(*(reinterpret_cast<const unsigned char *>(fieldPtr)));
    fieldPtr += 1;
    fspt.setOtherRCID(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;
    
    fspt.setORNT(*(reinterpret_cast<const unsigned char *>(fieldPtr)));    //Orientation: 1,2,255 = Forward, Reverse, NULL 
    fieldPtr += 1;
    fspt.setUSAG(*(reinterpret_cast<const unsigned char *>(fieldPtr)));    //Usage Indicator: 1,2,3, 255 = Ext., Int., DataLimit, NULL
    fieldPtr += 1;
    fspt.setMASK(*(reinterpret_cast<const unsigned char *>(fieldPtr)));    //Masking: 1,2,255 = Mask, Show, NULL
    fieldPtr += 1;

    if (fieldPtr > endPtr) throw QString("ERROR: FSPT Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);

    feat.getFsptVec().push_back(fspt);
    return 1+4+1+1+1;
}

//*****************************************************************************
/// Parse Vec.Rec Identifyer Field
/*!
* Important, Always the first field after "0001" Field, gives info about type of Record
****************************************************************************** */
void CellParser8211Dirty::parseVRIDField(const char * fieldPtr, int fieldLen, FieldVRID & vrid)
{
    const char * endPtr = fieldPtr + fieldLen;

    //**** 1. Record Name, maybe 110,120,130,140 = VI,VC,VE,VF ****
    vrid.setRCNM(*(reinterpret_cast<const unsigned char *>(fieldPtr))); 
    fieldPtr += 1;

    //**** 2. Record-id  ****
    vrid.setRCID(ISO8211::bytes2ulong(fieldPtr));
    fieldPtr += 4;  

    //**** 3. Record Version Ser. Num. - unimportant ****
    vrid.setRVER(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;

    //**** 4. Record Update Instruction = 1,2 or 3 = I,D or M
    vrid.setRUIN(*(reinterpret_cast<const unsigned char *>(fieldPtr)));

    if (fieldPtr > endPtr) throw QString("ERROR: VRID Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
/// 
/*!
* 
****************************************************************************** */
void CellParser8211Dirty::parseNodeRecord(Iso8211fieldIterator & fieldIt, NodeS57 & nodeRec)
{
    while (fieldIt.next())
    {
        const char * tag = 0;
        int fieldSize = 0;
        const char * field = fieldIt.current(&fieldSize, tag);
        
        if (strncmp(tag, "ATTV", 4) == 0)
        {
            parseATTVFields(field, fieldSize, nodeRec);
        }
        else if (strncmp(tag, "SG2D", 4) == 0)
        {
            parseSG2DField(field, fieldSize, nodeRec.getLat(), nodeRec.getLon()); 
        }
        else if (strncmp(tag, "VRPT", 4) == 0)
        {
            //** should not occure in Node !
            ++unusedVRPT;   
        }
        else
        {
            ++unusedSpatTag;  //** ???
        }
    }
}

void CellParser8211Dirty::parseEdgeRecord(Iso8211fieldIterator & fieldIt, EdgeS57 & edgeRec)
{
    while (fieldIt.next())
    {
        const char * tag = 0;
        int fieldSize = 0;
        const char * field = fieldIt.current(&fieldSize, tag);
        //probably,there are several vertices, so check SG2D first
        if (strncmp(tag, "SG2D", 4) == 0)
        {
            parseSG2DFields(field, fieldSize, edgeRec.getSG2Dvec());
        }
        else if (strncmp(tag, "ATTV", 4) == 0)
        {
            parseATTVFields(field, fieldSize, edgeRec);
        }
        //**rem: normally, an edge has at least 2 VRPT for its bounding nodes
        else if (strncmp(tag, "VRPT", 4) == 0)
        {
            int fieldCnt = fieldSize / 9;
            for (int i = 0; i < fieldCnt; ++i)
            {
                parseVRPTField(field, fieldSize, edgeRec);
                field += 9;
                fieldSize -= 9;
            }
        }
        else
        { 
            ++unusedSpatTag; //** ???
        }
    }
}

void CellParser8211Dirty::parseSndgRecord(Iso8211fieldIterator & fieldIt, SoundgS57 & sndgRec)
{
    while (fieldIt.next())
    {
        const char * tag = 0;
        int fieldSize = 0;
        const char * field = fieldIt.current(&fieldSize, tag);
        //probably,there are several vertices, so check SG2D first
        if (strncmp(tag, "SG3D", 4) == 0)
        {
            parseSG3DFields(field, fieldSize, sndgRec.getSG3Dvec()); 
        }
        else if (strncmp(tag, "ATTV", 4) == 0)
        {
            parseATTVFields(field, fieldSize, sndgRec);
        }
        else if (strncmp(tag, "VRPT", 4) == 0)
        { 
            ++unusedVRPT;   //** should not occure in Node !
        }
        else
        { 
            ++unusedSpatTag; //** ???
        }
    }
}

//*****************************************************************************
/// Parse a ATTV Attributes and put it into map
/*!
* Assuming that every attribute occures only once
****************************************************************************** */
void CellParser8211Dirty::parseATTVFields(const char * fieldPtr, int fieldLen, SpatialS57 & spRec)
{
    const char * allEndPtr = fieldPtr + fieldLen;
    while (fieldPtr < allEndPtr)
    {
        if (allEndPtr - fieldPtr == 2) 
        {
            throw QString("ERROR: ATTV Field to short: only 2 Bytes!");
        }
        unsigned short ATTL = ISO8211::bytes2ushort(fieldPtr);
        fieldPtr += 2;
        int valLen = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, allEndPtr);
        QString attrVal = QString::fromLatin1(fieldPtr, valLen);
        spRec.getAttributes()[ATTL] = attrVal;
        fieldPtr = ISO8211::findCharPosition(fieldPtr, ISO8211::FT, allEndPtr); //normally FT follows UT immediately, but: is it shure ??!!
        ++fieldPtr;
    }
}

//*****************************************************************************
/// Parse a VPPT Field - only Bounding nodes
/*!
* VRPT Field contains many interesting subfields, but most are unused in S-57 ENCs
* only pointer to Bounding Nodes are of interest
****************************************************************************** */
void CellParser8211Dirty::parseVRPTField(const char * fieldPtr, int fieldLen, EdgeS57 & edgeRec)
{
    const char * endPtr = fieldPtr + fieldLen;

    //**** 1st: NAME = Record-Name + Record-ID - Rec.Name must be 120 (Bounding-Node) ****
    unsigned char RCNM = *(reinterpret_cast<const unsigned char *>(fieldPtr));
    ++fieldPtr;
    unsigned long RCID = ISO8211::bytes2ulong(fieldPtr);
    fieldPtr += 4;

    //**** 2. Orientation - unimporant for S-57 ENCs ??? ****
#ifdef _DEBUG
    unsigned char ORNT = *(reinterpret_cast<const unsigned char *>(fieldPtr)); 
#endif
    fieldPtr += 1;

    //**** 3. Usage Indicator - unimporant for S-57 ENCs ****
#ifdef _DEBUG
    unsigned char USAG = *(reinterpret_cast<const unsigned char *>(fieldPtr));   
#endif
    fieldPtr += 1;

    //**** 4. Topology Indicatro - important for bouding nodes ****
    unsigned char TOPI = *(reinterpret_cast<const unsigned char *>(fieldPtr));   
    fieldPtr += 1;

    //**** 5. Mask: unimportant for S-57 ENCs ???? ****
#ifdef _DEBUG
    unsigned char MASK = *(reinterpret_cast<const unsigned char *>(fieldPtr));   
#endif
    fieldPtr += 1;

    //**** extract boudning nodes pointer ****
    if (RCNM == ISO8211::RCNM_VC)
    {
        if      (TOPI == 1) edgeRec.setStartNodeRecId(RCID);
        else if (TOPI == 2) edgeRec.setEndNodeRecId(RCID);
    }

    if (fieldPtr > endPtr) throw QString("ERROR: VRPT Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
/// Parse a  Field   //UPDATES only
/*!
* 
****************************************************************************** */
void CellParser8211Dirty::parseVRPCField(const char * fieldPtr, int fieldLen, FieldVRPC & vrpc)
{
    if (fieldLen == 0) fieldLen = vrpc.getIso8211Length();

    const char * endPtr = fieldPtr + fieldLen;
  
    vrpc.setVPUI(*(reinterpret_cast<const unsigned char *>(fieldPtr))); 
    fieldPtr += 1;

    vrpc.setVPIX(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;  

    vrpc.setNVPT(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;

    if (fieldPtr > endPtr) throw QString("ERROR: VRPC Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
/// Parse a  Field   //UPDATES only
/*!
* 
****************************************************************************** */
void CellParser8211Dirty::parseSGCCField(const char * fieldPtr, int fieldLen, FieldSGCC & sgcc)
{
    if (fieldLen == 0) fieldLen = sgcc.getIso8211Length();

    const char * endPtr = fieldPtr + fieldLen;
  
    sgcc.setCCUI(*(reinterpret_cast<const unsigned char *>(fieldPtr))); 
    fieldPtr += 1;

    sgcc.setCCIX(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;  

    sgcc.setCCNC(ISO8211::bytes2ushort(fieldPtr));
    fieldPtr += 2;  

    if (fieldPtr > endPtr) throw QString("ERROR: VRPC Field %1 bytes expected, but %2 extra bytes found!").arg(fieldLen).arg(fieldPtr - endPtr);
}

//*****************************************************************************
/// Parse a SG2D Field  , containing only ONE coordinate (Nodes)
/*!
* rem: Nodel ALWAYS have a coordinate
****************************************************************************** */
long CellParser8211Dirty::parseSG2DField(const char * fieldPtr, int fieldLen, double & lat, double & lon)
{
    if (fieldLen < 8) throw QString("ERROR: SG2D field to short: length = %1").arg(fieldLen);

    lat = ISO8211::bytes2long(fieldPtr) / facXY;
    fieldPtr += 4;
    lon = ISO8211::bytes2long(fieldPtr) / facXY;
    fieldPtr += 4;
    return 8;
}

//*****************************************************************************
/// Parse a SG2D Field vector  , containing SEVERAL coordinates (Edges)
/*!
* rem: a Edge might have an EMPTY coordinate field
****************************************************************************** */
void CellParser8211Dirty::parseSG2DFields(const char * fieldPtr, int fieldLen, std::vector< double > & latLonVec)
{
    if (fieldLen == 0 || fieldLen == 1) return; //empty coordinate field

    const char * endPtr = fieldPtr + fieldLen; //rem: points to the FT
    int count = fieldLen / 4 ;
    if (fieldLen < 8){
        throw QString("ERROR: SG2D field to short, length = %1!").arg(fieldLen);}
    if (count %2 != 0) throw QString("ERROR: SG2D has %2 values - even number of values expected !").arg(count);

    latLonVec.reserve(count);  //prevent reallocation - big probem with big vectors!    
    for (int i = 0; i < count; ++i)
    {
        double val = ISO8211::bytes2long(fieldPtr) / facXY;
        fieldPtr += 4;
        latLonVec.push_back(val);
    } 
}

//*****************************************************************************
/// Parse a SG3D Field vector, (used for Soundings)
/*!
* rem: Soundings may have one or more coordinates, but should not be empty
****************************************************************************** */
void CellParser8211Dirty::parseSG3DFields(const char * fieldPtr, int fieldLen, std::vector< double > & latLonZVec)
{
    const char * endPtr = fieldPtr + fieldLen; //rem: points to the FT
    int rounds = fieldLen / 12 ;
    int valCount = fieldLen / 4;
    if (fieldLen < 12){
        throw QString("ERROR: SG3D field to short: only %1 values (minmum is 3)").arg(valCount);}
    if (valCount %3 != 0) throw QString("ERROR: SG3D does not have triples of values (%1 values)!").arg(valCount);

    latLonZVec.reserve(valCount);//prevent reallocation - big probem - sounding clusters may be VERY big!    

    for (int i = 0; i < rounds; ++i)
    {
        latLonZVec.push_back(ISO8211::bytes2long(fieldPtr) / facXY); //Lat
        fieldPtr += 4;
        latLonZVec.push_back(ISO8211::bytes2long(fieldPtr) / facXY); //Lon
        fieldPtr += 4;
        latLonZVec.push_back(ISO8211::bytes2long(fieldPtr) / facZ); //depth
        fieldPtr += 4;
    }
}


