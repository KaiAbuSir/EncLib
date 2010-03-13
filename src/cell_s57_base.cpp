//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_s57_base.h"

#include <QtCore/QMap>

#include <algorithm>

#include "cell_parser_iso8211dirty4base.h"
#include "cell_parser_iso8211dirty4updt.h"
#include "cell_writer_iso8211dirty.h"
#include "s57updater.h"
#include "cell_check_s57.h"

using namespace Enc;


CellS57_Base::CellS57_Base() : CellS57_Header()
{}

CellS57_Base::~CellS57_Base()
{}

//******************************************************************************
///
//**************************************************************************** */
void CellS57_Base::clear()
{
    CellS57_Header::clear();

    //** clear container which hold pointer, which must be deleted, too **
    for (std::map< unsigned long, FeatureS57 *>::iterator fit = features.begin(); fit != features.end(); ++fit) delete fit->second;
    features.clear();

    for (std::map< unsigned long, NodeS57 *>::iterator inIt =  iNodes.begin(); inIt != iNodes.end(); ++inIt) delete inIt->second; 
    iNodes.clear();

    for (std::map< unsigned long, BoundNodeS57 *>::iterator bnIt = bNodes.begin(); bnIt != bNodes.end(); ++bnIt) delete bnIt->second;
    bNodes.clear();

    for (std::map< unsigned long, SoundgS57 *>::iterator sIt = soundings.begin(); sIt != soundings.end(); ++sIt) delete sIt->second;
    soundings.clear();

    for (std::map< unsigned long, EdgeS57 *>::iterator eIt = edges.begin(); eIt != edges.end(); ++eIt) delete eIt->second;
    edges.clear();

    //** clear containers which hold handles, only **
    foid2rcid.clear();
    newFeatures.clear();
    newEdges.clear();
}

//******************************************************************************
///
//**************************************************************************** */
int CellS57_Base::recordCount() const
{
     int recCnt = features.size() + iNodes.size() + bNodes.size() + soundings.size() + edges.size();
     if (dsid.getPROF() == 1) recCnt += 3; //DDR and 2 Cellheader Records (Base Cell)
     else  recCnt += 2; //DDR and ONE Cellheader record (Update)
     return recCnt;
}

//******************************************************************************
///
//**************************************************************************** */
void CellS57_Base::updateDSSIrecordCounts()
{
    dssi.setNOIN(iNodes.size() + soundings.size());
    dssi.setNOCN(bNodes.size());
    dssi.setNOED(edges.size());
    dssi.setNOFA(0);
}

//******************************************************************************
/// Conveniance Method: Fill update cell by parsing a Base-File (*.000 file)
/*! File is paresed completely, except the DDR
 **************************************************************************** */
void CellS57_Base::parseISO8211(QString fileNamePath)
{
    CellParser8211Dirty4Base s57parser(this, CellParser8211::ParseAll);
    s57parser.parseS57Cell(fileNamePath);
    processBackwardPointers();
}

//******************************************************************************
/// Write contents to File
//**************************************************************************** */
void CellS57_Base::writeISO8211(QString fileNamePath) const
{
    CellWriter8211Dirty s57writer(this);
    s57writer.writeS57Cell(fileNamePath);
}

//******************************************************************************
/// Apply several updates at once (files need not be in right order)
/*!
 * If there is only ONE update-File, extension does not matter. But otherwise,
 * they must have the right extension to make ordering possible: 001,002,003...
 ***************************************************************************** */
void CellS57_Base::applyUpdates(QStringList updateCells)
{
    if (updateCells.empty()) return;
    if (updateCells.size() > 1)
    {
        QMap<int, QString>cellsMap;
        for (QStringList::iterator liIt = updateCells.begin(); liIt != updateCells.end(); ++liIt)
        {
            bool extOk;
            int updtNum = (*liIt).right(3).toUInt(&extOk);
            if (extOk) cellsMap[updtNum] = *liIt;
            else throw QString("ERROR: Cannot apply update: %1 Wrong Extension!").arg(*liIt);
        }
        updateCells.clear();
        for (QMap<int, QString>::iterator mIt= cellsMap.begin(); mIt != cellsMap.end(); ++mIt) updateCells.append(mIt.value());
    }

    S57Updater updater;
    updater.setCell(this);

    CellS57_Update updtCell(dspm); //rem: update-cell needs DSPM Field from base-cell!
    CellParser8211Dirty4Updt updtParser(&updtCell);

    QStringList::iterator updtIt = updateCells.begin();
    for (; updtIt != updateCells.end(); ++updtIt)
    {
        if (updtIt != updateCells.begin()) updtCell.clearExceptDSPM();
        updtParser.parseS57Cell(*updtIt);
        updater.applyUpdate(&updtCell);
#ifdef _DEBUG
        CellCheckS57 cellChecker(this);
        CheckResult checkResult = cellChecker.check();
        if (checkResult.errCnt() || checkResult.outsidePointer.size())
        {
            if (checkResult.errCnt()) qWarning("ERROR: %d update errros after applying %s", checkResult.errCnt(), (*updtIt).toLatin1().data());
            QStringList errMsg;
            checkResult.makeMessage(errMsg);
            printf("\n%s", errMsg.join("\n").toLatin1().data());
        }
#endif
    }
}

//******************************************************************************
///
//**************************************************************************** */
bool CellS57_Base::recordExists(unsigned char RCNM, unsigned long RCID) const
{
    if (RCNM == ISO8211::RCNM_FE) return (features.find(RCID) != features.end());//Feature
    else if (RCNM == ISO8211::RCNM_VI) //iNode
    {
        if (iNodes.find(RCID) != iNodes.end()) return true;       
        if (soundings.find(RCID) != soundings.end()) return true;  
        return false;
    }
    else if (RCNM == ISO8211::RCNM_VC) return (bNodes.find(RCID) != bNodes.end());//BNode
    else if (RCNM == ISO8211::RCNM_VE) return (edges.find(RCID) != edges.end()); //Edge   
    return false;
}

//*****************************************************************************
/// Delete a Feature or Spatial Record, but dont care for orphans or for backward pointer
/*!
* HINT: 1) The Record-object itself is deleted, than 2) the pointer is removed from its container
****************************************************************************** */    
void CellS57_Base::deleteRecordStupid(unsigned char RCNM, unsigned long RCID)
{
    if (RCNM == ISO8211::RCNM_FE) //Feature
    {
        std::map< unsigned long, FeatureS57 *>::iterator fIt = features.find(RCID);
        if (fIt != features.end())
        {   delete fIt->second;
            features.erase(fIt);
        }
    }
    else if (RCNM == ISO8211::RCNM_VI) //iNode
    {
        std::map< unsigned long, NodeS57 *>::iterator inIt = iNodes.find(RCID);
        if (inIt != iNodes.end())
        {
            delete inIt->second;
            iNodes.erase(inIt);
        }
        else
        {
            std::map< unsigned long, SoundgS57 *>::iterator sIt = soundings.find(RCID);
            if (sIt != soundings.end())
            {
                delete sIt->second;
                soundings.erase(sIt);
            }
        }
    }
    else if (RCNM == ISO8211::RCNM_VC) //BNode
    {
        std::map< unsigned long, BoundNodeS57 *>::iterator bnIt = bNodes.find(RCID);
        if (bnIt != bNodes.end())
        {
            delete bnIt->second;
            bNodes.erase(bnIt);  
        }
    }
    else if (RCNM == ISO8211::RCNM_VE) //Edge
    {
        std::map< unsigned long, EdgeS57 *>::iterator eIt = edges.find(RCID);
        if (eIt != edges.end())
        {
            delete eIt->second;
            edges.erase(eIt);      
        }
    }
}

//*****************************************************************************
/// Delete a Feature or Spatial Record, and delete backward pointer if needed
/*!
* But: - does not care for orphans
*      - does not care for the original forward pointers of other records
* Mainly usefull to perform delete-instructions in Update Cells
****************************************************************************** */ 
void CellS57_Base::deleteRecordAndBackPointer(unsigned char RCNM, unsigned long RCID)
{
    if (RCNM == ISO8211::RCNM_FE) 
    {
        FeatureS57 & delFeat = getFeature(RCID);
        //** remove all Spat->Feat back-pointer **
        std::vector<SpatialS57 *> featSpatials;
        getSpatialsForFeature(delFeat, featSpatials);
        for (uint gI = 0; gI < featSpatials.size(); ++gI)
        {
            featSpatials[gI]->remFeature(RCID);
        }

        //** delete Feat->Feat back-pointer
        std::multimap < LongNAMe, unsigned long >::iterator it;
        const std::vector < FieldFFPT > & relations = delFeat.getFfptVec();
        for (uint i =0; i < relations.size(); ++i)
        {
            LongNAMe relatedFoid = relations[i].getLNAM();
            for (it = foid2rcid.lower_bound(relatedFoid); it != foid2rcid.upper_bound(relatedFoid); ++it);
            {
                FeatureS57 & relatedFeat = getFeature(it->second);
                relatedFeat.remFfptBackPointer(RCNM);
            }
        }

        //** delete from FOID-map
        LongNAMe myFoid = delFeat.getLongNAMe();
        for (it = foid2rcid.lower_bound(myFoid); it != foid2rcid.upper_bound(myFoid); it = foid2rcid.erase(it));
 
    }
    //** Nodes can be deleted directly: the backward pointer are part of the Spatial, and will be deleted automaically ** 
    else if (RCNM == ISO8211::RCNM_VI || RCNM == ISO8211::RCNM_VC)
    {
        ;
    }
    else if (RCNM == ISO8211::RCNM_VE)
    {
        const EdgeS57 & delEdge = getEdge(RCID);
        //** delete Node -> Edge back-pointer
        unsigned long startRcid = delEdge.getStartNodeRecId();
        unsigned long endRcid   = delEdge.getEndNodeRecId();
        if (startRcid)
        {
            getBNode(startRcid).remEdge(RCID);
        }
        if (endRcid && startRcid != endRcid)
        {   
            getBNode(endRcid).remEdge(RCID);
        }
    }

    deleteRecordStupid(RCNM, RCID);
}

//******************************************************************************
/// Delete a feature record and all spatials that would become orphaned
//**************************************************************************** */
void CellS57_Base::deleteFeatureWithOrphans(unsigned long RCID)
{
    FeatureS57 & feat2kill = getFeature(RCID);
    std::vector<const SpatialS57 *> spatials;
    getSpatialsForFeature(feat2kill,  spatials);
    for (uint i=0; i < spatials.size(); ++i)
    {
        const SpatialS57 * spat = spatials[i];
        const std::vector <unsigned long> & featRCIDs = spat->getFeatures();
        if (featRCIDs.size() == 1)
        {
            if (featRCIDs[0] == RCID)
            {
                deleteSpatialWithOrphanSpatials(spat->getVRID().getRCNM(), spat->getVRID().getRCID());
            }
            else
            {
                throw QString("%1 ERROR: pointer to feature %2 out of sync!").arg(spat->getNameASCII()).arg(feat2kill.getNameASCII());
            }
        }
    }
}

//******************************************************************************
/// Delete a Spatial, in case of Edge, delete BoundingNodes if orphaned 
//**************************************************************************** */
void CellS57_Base::deleteSpatialWithOrphanSpatials(unsigned char RCNM, unsigned long RCID)
{
    if (RCNM == ISO8211::RCNM_VE)
    {
        const EdgeS57 & edge = getEdge(RCID);
        
        //**** in any case: remove backward-pointer from Bounding-nodes ****
        //**** if Bounding Nodes became orphaned, kill them ****
        unsigned long rcidStart = edge.getStartNodeRecId();
        unsigned long rcidEnd = edge.getEndNodeRecId();
        if (rcidStart)
        {
            BoundNodeS57 & startNode = getBNode(rcidStart);
            startNode.remEdge(RCID);
            if (startNode.isOrphan())
            {
                deleteRecordStupid(ISO8211::RCNM_VC, rcidStart); //here, start-node becomes invalid!
            }     
        }
        if (rcidStart != rcidEnd)
        {
            BoundNodeS57 & endNode = getBNode(rcidEnd);
            endNode.remEdge(RCID);
            if (endNode.isOrphan())
            {
                deleteRecordStupid(ISO8211::RCNM_VC, rcidEnd); //now, end-node becomes invalid!
            } 
        }
    }
    deleteRecordStupid(RCNM, RCID);
}

//******************************************************************************
///
//**************************************************************************** */
void CellS57_Base::getSpatialsForFeature(const FeatureS57 & feat, std::vector<const SpatialS57 *> & spatials) const
{
    const std::vector < FieldFSPT > & fspts = feat.getFsptVec();
    for (uint i=0; i<fspts.size(); ++i)
    {
        spatials.push_back(&(getSpatial(fspts[i].getOtherRCNM(), fspts[i].getOtherRCID())));
    }
}

void CellS57_Base::getSpatialsForFeature(const FeatureS57 & feat, std::vector<SpatialS57 *> & spatials)
{
    const std::vector < FieldFSPT > & fspts = feat.getFsptVec();
    for (uint i=0; i<fspts.size(); ++i)
    {
        spatials.push_back(&(getSpatial(fspts[i].getOtherRCNM(), fspts[i].getOtherRCID())));
    } 
}

//******************************************************************************
///
//**************************************************************************** */
const SpatialS57 & CellS57_Base::getSpatial(unsigned int rcnm, unsigned long rcid) const
{
    if (rcnm == ISO8211::RCNM_VI)  //VI (node or sounding)
    {
        if (iNodes.find(rcid) != iNodes.end()) return getINode(rcid);
        else return getSounding(rcid);
    }
    else if (rcnm == ISO8211::RCNM_VC) return getBNode(rcid); //VC
    else if (rcnm == ISO8211::RCNM_VE) return getEdge(rcid); //VE
    throw QString("ERROR: Cannot get Spatial %1: RCNM invalid!").arg(ISO8211::makeRecNameASCII(rcnm, rcid));
}

SpatialS57 & CellS57_Base::getSpatial(unsigned int rcnm, unsigned long rcid)
{
    if (rcnm == ISO8211::RCNM_VI)  //VI (node or sounding)
    {
        if (iNodes.find(rcid) != iNodes.end()) return getINode(rcid);
        else return getSounding(rcid);
    }
    else if (rcnm == ISO8211::RCNM_VC) return getBNode(rcid); //VC
    else if (rcnm == ISO8211::RCNM_VE) return getEdge(rcid); //VE
    throw QString("ERROR: Cannot get Spatial: RCNM invalid!").arg(ISO8211::makeRecNameASCII(rcnm, rcid));
}

//******************************************************************************
///
//**************************************************************************** */
FeatureS57 & CellS57_Base::getFeature(unsigned long RCID)
{
    std::map< unsigned long, FeatureS57 *>::iterator It = features.find(RCID);  
    if (It == features.end()) throw QString("ERROR: Feature does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_FE, RCID));
    return *It->second;
}

//******************************************************************************
///
//**************************************************************************** */
NodeS57 & CellS57_Base::getINode(unsigned long RCID)
{
    std::map< unsigned long, NodeS57 *>::iterator It = iNodes.find(RCID); 
    if (It == iNodes.end()) throw QString("ERROR: Isolated Node does not exists: %1!").arg(ISO8211::makeRecNameASCII(110, RCID));
    return *It->second;
}

//******************************************************************************
///
//**************************************************************************** */
BoundNodeS57 & CellS57_Base::getBNode(unsigned long RCID)
{
    std::map< unsigned long, BoundNodeS57 *>::iterator It =  bNodes.find(RCID); 
    if (It == bNodes.end()) throw QString("ERROR: Bounding Node does not exists: %1!").arg(ISO8211::makeRecNameASCII(120, RCID));
    return *It->second;
}

//******************************************************************************
///
//**************************************************************************** */
SoundgS57 & CellS57_Base::getSounding(unsigned long RCID)
{
    std::map< unsigned long, SoundgS57 *>::iterator It = soundings.find(RCID); 
    if (It == soundings.end()) throw QString("ERROR: Sounding does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VI, RCID));
    return *It->second;
}

EdgeS57 & CellS57_Base::getEdge(unsigned long RCID)
{
    std::map< unsigned long, EdgeS57 *>::iterator It = edges.find(RCID);  
    if (It == edges.end()) throw QString("ERROR: Edge does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VE, RCID));
    return *It->second;
}

const FeatureS57 & CellS57_Base::getFeature(unsigned long RCID) const
{
    std::map< unsigned long, FeatureS57 *>::const_iterator It = features.find(RCID);  
    if (It == features.end()) throw QString("ERROR: Feature does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_FE, RCID));
    return *It->second;
}
const NodeS57 & CellS57_Base::getINode(unsigned long RCID) const
{
    std::map< unsigned long, NodeS57 *>::const_iterator It = iNodes.find(RCID); 
    if (It == iNodes.end()) throw QString("ERROR: Isolated Node does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VI, RCID));
    return *It->second;
}
const BoundNodeS57 & CellS57_Base::getBNode(unsigned long RCID) const
{
    std::map< unsigned long, BoundNodeS57 *>::const_iterator It =  bNodes.find(RCID); 
    if (It == bNodes.end()) throw QString("ERROR: Bounding Node does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VC, RCID));
    return *It->second;
}
const SoundgS57 & CellS57_Base::getSounding(unsigned long RCID) const
{
    std::map< unsigned long, SoundgS57 *>::const_iterator It = soundings.find(RCID); 
    if (It == soundings.end()) throw QString("ERROR: Sounding does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VI, RCID));
    return *It->second;
}
const EdgeS57 & CellS57_Base::getEdge(unsigned long RCID) const
{
    std::map< unsigned long, EdgeS57 *>::const_iterator It = edges.find(RCID);  
    if (It == edges.end()) throw QString("ERROR: Edge does not exists: %1!").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VE, RCID));
    return *It->second;
}

//******************************************************************************
///
//**************************************************************************** */
void CellS57_Base::addFeature(const FeatureS57 & feat2add)
{
    FeatureS57 * newFeat = new FeatureS57(feat2add);
    //** check/set Record-ID **
    if (newFeat->getFRID().getRCID() == 0)
    {
        unsigned long maxRCID = getMaxRCID4FE();
        ++maxRCID;
        newFeat->getFRID().setRCID(maxRCID);
        features[maxRCID] = newFeat;
    }
    else
    {
        features[newFeat->getFRID().getRCID()] = newFeat;
    }
    newFeatures.push_back(newFeat->getFRID().getRCID());
}

//******************************************************************************
///
//**************************************************************************** */
void CellS57_Base::addSpatial(const SpatialS57 & spat)
{
    if (spat.getType() == Record8211::Node || (spat.getType() == Record8211::NodeUPDT && spat.getVRID().getRCNM() == ISO8211::RCNM_VI))
    {
        addINode(static_cast<const NodeS57 &>(spat));
    }
    else if (spat.getType() == Record8211::BNode || (spat.getType() == Record8211::NodeUPDT && spat.getVRID().getRCNM() == ISO8211::RCNM_VC))
    {
        addBNode(static_cast<const NodeS57 &>(spat));
    }
    else if (spat.getType() == Record8211::Edge || spat.getType() == Record8211::EdgeUPDT)
    {
        addEdge(static_cast<const EdgeS57 &>(spat));
    }
    else if (spat.getType() == Record8211::Sounding || spat.getType() == Record8211::SoundingUPDT)
    {
        addSoundg(static_cast<const SoundgS57 &>(spat));
    }
}
void CellS57_Base::addINode(const NodeS57 & node2add)
{
    NodeS57 * newINode = new NodeS57(node2add);
    if (!setRCIDifNeeded(*newINode))
    {
        if (iNodes.find(newINode->getVRID().getRCID()) != iNodes.end()) throw QString("ERROR: Cannot add isolated node record: RCID already exists: %1").arg(newINode->getNameASCII());
    }
    iNodes[newINode->getVRID().getRCID()] = newINode;       ///KEY: RecordId of an Isolated Node ("VI")
}
void CellS57_Base::addBNode(const NodeS57 & bNode2add)
{
    BoundNodeS57 * newBNode = new BoundNodeS57(bNode2add);
    if (!setRCIDifNeeded(*newBNode))
    {
        if (bNodes.find(newBNode->getVRID().getRCID()) != bNodes.end()) throw QString("ERROR: Cannot add bounding node record: RCID already exists: %1").arg(newBNode->getNameASCII());
    }
    bNodes[newBNode->getVRID().getRCID()] = newBNode;  
}
void CellS57_Base::addSoundg(const SoundgS57 & sndg2add)
{
    SoundgS57 * newSndg = new SoundgS57(sndg2add);
    if (!setRCIDifNeeded(*newSndg))
    {
        if (soundings.find(newSndg->getVRID().getRCID()) != soundings.end()) throw QString("ERROR: Cannot add bounding node record: RCID already exists: %1").arg(newSndg->getNameASCII());
    }
    soundings[newSndg->getVRID().getRCID()] = newSndg;  
}

//******************************************************************************
///
//**************************************************************************** */
void CellS57_Base::addEdge(const EdgeS57 & edge2add)
{
    EdgeS57 * newEdge = new EdgeS57(edge2add);
    if (!setRCIDifNeeded(*newEdge))
    {
        if (edges.find(newEdge->getVRID().getRCID()) != edges.end()) throw QString("ERROR: Cannot add edge record: RCID already exists: %1").arg(edge2add.getNameASCII());
    }
    edges[newEdge->getVRID().getRCID()] = newEdge;
    newEdges.push_back(newEdge->getVRID().getRCID());
}

//******************************************************************************
/// Set the Record-ID if not yet done - returns true if RCID was really set
//**************************************************************************** */
bool CellS57_Base::setRCIDifNeeded(SpatialS57 & spatial)
{
    if (spatial.getVRID().getRCID() > 0) return false;
    unsigned char rcnm = spatial.getVRID().getRCNM();
    
    unsigned long rcid = 0;
    if      (rcnm == ISO8211::RCNM_VI) rcid = getMaxRCID4VI();//Isolated Node
    else if (rcnm == ISO8211::RCNM_VC) rcid = getMaxRCID4VC();//Connected Node
    else if (rcnm == ISO8211::RCNM_VE) rcid = getMaxRCID4VE();//Edge
    else
    {
        throw QString("ERROR: Cannot set RCID for Spatial: No valid RCNM: %1").arg(rcnm);
    }
    ++rcid;
    spatial.getVRID().setRCID(rcid);
    return true;
}

//******************************************************************************
///Get the highest Record-ID of Feature Records "FE"
//**************************************************************************** */
unsigned long CellS57_Base::getMaxRCID4FE() const
{
    if (features.rbegin() == features.rend()) return 0;
    return features.rbegin()->first;
}

//******************************************************************************
///Get the highest Record-ID of Edge Records "VE"
//**************************************************************************** */
unsigned long CellS57_Base::getMaxRCID4VE() const
{
    if (edges.rbegin() == edges.rend()) return 0;
    return edges.rbegin()->first;
}

//******************************************************************************
///Get the highest Record-ID of Isolated Nodes Records "VE"
//**************************************************************************** */
unsigned long CellS57_Base::getMaxRCID4VI() const
{
    unsigned long maxId = 0;
    if (iNodes.rbegin() != iNodes.rend()) maxId = iNodes.rbegin()->first;
    if (soundings.rbegin() != soundings.rend() && soundings.rbegin()->first > maxId) maxId = soundings.rbegin()->first;
    return maxId;
}

//******************************************************************************
///Get the highest Record-ID of Connected Nodes Records "VC"
//**************************************************************************** */
unsigned long CellS57_Base::getMaxRCID4VC() const
{
    if (bNodes.rbegin() == bNodes.rend()) return 0;
    return bNodes.rbegin()->first;
}

//*****************************************************************************
/// Create all the Backward Pointers not stored in S-57 Cells
/*!
* In S-57, Pointers only point into one Direction, e.g. from Feature->Spatial ...etc
* but for fast access we need pointers in both directions:
* So here, for every pointer, the backward pointer is cached
*
* IMPORTANT: This Method should be called only once, after the cell is read
****************************************************************************** */    
void CellS57_Base::processBackwardPointers()
{
    //**** check/set Spatial2Feature Pointers, feat2feat pointers, and find orphaned Features ****
    std::map< unsigned long, FeatureS57 *>::iterator featIt ;  
    for (featIt =  features.begin(); featIt != features.end(); ++featIt)
    {
        processBackwardPointer4feature(featIt->second);
    }

    //**** set Node To Edge Pointers and find broken Pointers ****
    std::map< unsigned long, EdgeS57 *>::iterator edgeIt;
    for (edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)     
    {
        const EdgeS57 * edge = edgeIt->second;
        processBackwardPointer4edge(edge);
    }
}

//*****************************************************************************
/// Create the missing Backward Pointers for newly added Records
/*!
* Usefull if the cell is updated, and only few back-pointers need to be processed
****************************************************************************** */  
void CellS57_Base::processBackwardPointers4newRecords()
{
    //**** calc backward pointer for all features ****
    for (uint iF = 0; iF < newFeatures.size(); ++iF)
    {
        FeatureS57 & feat = getFeature(newFeatures[iF]);
        processBackwardPointer4feature(&feat);

    }
    //**** calc backward pointer (node2edge) for all edges ****
    for (uint iE = 0; iE < newEdges.size(); ++iE)
    {        
        EdgeS57 & edge = getEdge(newEdges[iE]);
        processBackwardPointer4edge(&edge);
    }

    newFeatures.clear();
    newEdges.clear();
}

//*****************************************************************************
/// Store Feature-Backward-Pointers in Records to which "feature" points 
/*!
* At the same time, broken pointers will be detected
****************************************************************************** */    
void CellS57_Base::processBackwardPointer4feature(const FeatureS57 * feature)
{
    unsigned long rcid = feature->getFRID().getRCID();
    const std::vector <  FieldFSPT > & FSPTvec = feature->getFsptVec();
    unsigned int i=0;
    for (; i < FSPTvec.size(); ++i)
    {
        SpatialS57 * spatial = 0;
        //** special: RCNM 110 = Isolated Node: may be iNode or iNode-cluster(Sounding) -> check both ** 
        if(FSPTvec[i].getOtherRCNM() == 110)
        {
            std::map< unsigned long, NodeS57 *>::iterator nIt = iNodes.find(FSPTvec[i].getOtherRCID());
            if (nIt != iNodes.end()) spatial = nIt->second;

            if (spatial == 0)
            {
                std::map< unsigned long, SoundgS57 *>::iterator sIt = soundings.find(FSPTvec[i].getOtherRCID());
                if (sIt != soundings.end()) spatial = sIt->second;
            }
        }
        else if (FSPTvec[i].getOtherRCNM() == 120)
        {
            std::map< unsigned long, BoundNodeS57 *>::iterator bIt = bNodes.find(FSPTvec[i].getOtherRCID());
            if (bIt != bNodes.end()) spatial = bIt->second;
        }
        else if (FSPTvec[i].getOtherRCNM() == ISO8211::RCNM_VE)
        {
            std::map< unsigned long, EdgeS57 *>::iterator eIt = edges.find(FSPTvec[i].getOtherRCID());
            if (eIt != edges.end()) spatial = eIt->second;
        }
        //** remeber broken feature2spatial pointer **
        if (spatial == 0)
        {
            checkResult.brokenFeat2Spatial.push_back(std::pair<unsigned long, RecNAME>(rcid, makeRecName(FSPTvec[i].getOtherRCNM(), FSPTvec[i].getOtherRCID())));
        }
        else 
        {
            spatial->addFeature(rcid);
        }
    }
    //**** check if pointers are missing altogether (orphan) ****
    if ( i == 0 && (feature->getFRID().getPRIM() == 1 || feature->getFRID().getPRIM() == 2 || feature->getFRID().getPRIM() == 3))
    {
        checkResult.orphanFeatures.push_back(rcid);
    }

    //**** calculate the feat2feat backward pointer - if possible ****
    //**** rem: FOID might point to feature in other cell, or even to several Features in same cell
    const std::vector < FieldFFPT > & FFPTvec = feature->getFfptVec();
    for (uint i=0; i < FFPTvec.size(); ++i)
    {
        std::multimap < LongNAMe, unsigned long >::const_iterator it = foid2rcid.lower_bound(FFPTvec[i].getLNAM());
        for(; it != foid2rcid.upper_bound(FFPTvec[i].getLNAM()); ++it)
        {
            FeatureS57 & targetFeat = getFeature(it->second);
            targetFeat.addFfptBackPointer(rcid); 
        }
    }
}

//*****************************************************************************
/// Store Edge-Pointers the Connected Nodes which are bounding the "edge" 
/*!
* At the same time, broken Edge->Node pointers will be detected
****************************************************************************** */    
void CellS57_Base::processBackwardPointer4edge(const EdgeS57 * edge)
{
    unsigned long rcid = edge->getVRID().getRCID();
    if (edge->getStartNodeRecId())
    {
        std::map< unsigned long, BoundNodeS57 *>::iterator bNodeIt = bNodes.find(edge->getStartNodeRecId());
        if (bNodeIt == bNodes.end())
        {
            checkResult.brokenEdge2Node.push_back(RCIDPair(rcid, edge->getStartNodeRecId()));
        }
        else
        {
            bNodeIt->second->addEdge(rcid);
        }
    }
    else
    {
        checkResult.edgesMissingBNodes.push_back(rcid);
    }
    if (edge->getStartNodeRecId() != edge->getEndNodeRecId())
    {
        if (edge->getEndNodeRecId())
        {
            std::map< unsigned long, BoundNodeS57 *>::iterator bNodeIt = bNodes.find(edge->getEndNodeRecId());
            if (bNodeIt == bNodes.end())
            {
                checkResult.brokenEdge2Node.push_back(RCIDPair(rcid, edge->getEndNodeRecId()));
            }
            else
            {
                bNodeIt->second->addEdge(rcid);
            }
        }
        else
        {
            checkResult.edgesMissingBNodes.push_back(rcid);
        }
    }
}

