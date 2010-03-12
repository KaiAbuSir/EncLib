
#include "s57updater.h"

using namespace Enc;

S57Updater::S57Updater()
{

}
S57Updater::~S57Updater()
{}

void S57Updater::setCell(CellS57_Base * baseCellS57) 
{
    reIssue = baseCellS57;
}

//******************************************************************************
/// Apply Update, if possible
/*!
* 
***************************************************************************** */
void S57Updater::applyUpdate(const CellS57_Update * updtCell)
{
    //**** check if update fits ****
    if (!reIssue) throw "ERROR: Cannot apply update: No base cell loaded!";
    if (!updtCell) throw QString("ERROR: Cannot apply update to: No update cell loaded!").arg(reIssue->getDsid().getDSNM());
    if (reIssue->getDsid().getDSNM().left(9).toUpper() != updtCell->getDsid().getDSNM().left(9).toUpper())
    {
        throw QString("ERROR: Cannot apply update %1 to base %2: Different Filenames!").arg(updtCell->getDsid().getDSNM()).arg(reIssue->getDsid().getDSNM());
    }
    if (reIssue->getDsid().getEDTN() != updtCell->getDsid().getEDTN())
    {
        throw QString("ERROR: Cannot apply update: %1 Edition number does not fit: %1 != %2").arg(updtCell->getDsid().getDSNM()).arg(reIssue->getDsid().getEDTN()).arg(updtCell->getDsid().getEDTN());
    }
    if (reIssue->getDsid().getUPDN() +1 != updtCell->getDsid().getUPDN())
    {
        throw QString("ERROR: Cannot apply update: %1 Updat numbers no sequential: %1 -> %2").arg(updtCell->getDsid().getDSNM()).arg(reIssue->getDsid().getUPDN()).arg(updtCell->getDsid().getUPDN());
    }

    //**** apply all feature updates ****
    const std::vector< FeatureS57_Updt *> & updtFeatures = updtCell->getFeatures(); 
    std::vector< FeatureS57_Updt *>::const_iterator uFtIt;
    for (uFtIt = updtFeatures.begin(); uFtIt != updtFeatures.end(); ++uFtIt)
    {
        applyUpdtFeature(**uFtIt);
    }

    //**** apply all spatial updates ****
    const std::vector< SpatialS57 *> &  spatials = updtCell->getSpatials();     
    std::vector< SpatialS57 * >::const_iterator spIt;
    for (spIt = spatials.begin(); spIt != spatials.end(); ++spIt)
    {
        applyUpdtSpatial(*spIt);
    }

    //**** Update the Cell-Header ****
    reIssue->getDsid().setUPDN(updtCell->getDsid().getUPDN()); //set new Update Number
    reIssue->getDsid().setUADT(updtCell->getDsid().getISDT()); //the update-issue-date becomes the update-date
    reIssue->updateDSSIrecordCounts();

    reIssue->processBackwardPointers4newRecords();
}

//******************************************************************************
/// Apply one Feature Update Record to dest Cell
/*!
* 
***************************************************************************** */
void S57Updater::applyUpdtFeature(const FeatureS57_Updt & updtFeat)
{
    //**** check what to do at all ****
    bool featExists = reIssue->recordExists(updtFeat.getFRID().getRCNM(), updtFeat.getFRID().getRCID());
    if (updtFeat.getFRID().getRUIN() == 1) //insert Feat into ReIssue
    {
        if (featExists) 
        {
            throw QString("ERROR: Cannot insert Feature: Feature already exists: %1").arg(updtFeat.getNameASCII());
        }
        reIssue->addFeature(updtFeat);
    }
    else if (updtFeat.getFRID().getRUIN() == 2) //Delete Feat from ReIssue
    {
        if (!featExists) 
        {
            throw QString("ERROR: Cannot delete Feature: Feature does not exists: %1").arg(updtFeat.getNameASCII());
        }
        reIssue->deleteRecordAndBackPointer(updtFeat.getFRID().getRCNM(), updtFeat.getFRID().getRCID());
    }
    else if (updtFeat.getFRID().getRUIN() == 3) //Modifiy Feat in Reissue
    {
        if (!featExists) 
        {
            throw QString("ERROR: Cannot modify Feature: Feature does not exists: %1").arg(updtFeat.getNameASCII());
        }
        modifyFeature(reIssue->getFeature(updtFeat.getFRID().getRCID()), updtFeat);
    }
    else
    {
        throw QString("ERROR: Invalid feature update instruction (RUIN =%1) in %2").arg(updtFeat.getFRID().getRUIN()).arg(updtFeat.getNameASCII());
    }
}

//******************************************************************************
/// Does all necessary Modifications to update a Feature (except Delete/Insert whole Feature)
/*!
* 
***************************************************************************** */
void S57Updater::modifyFeature(FeatureS57 & oldFeat, const FeatureS57_Updt & updtFeat)
{
    //**** check Record Version, new Version must be oldVersion +1 ****
    if (oldFeat.getFRID().getRVER() +1 != updtFeat.getFRID().getRVER())
    {
        throw QString("%1 ERROR: Cannot update: Record Version does not match: old=%2 new=%3").arg(oldFeat.getNameASCII()).arg(oldFeat.getFRID().getRVER()).arg(updtFeat.getFRID().getRVER());
    }
    oldFeat.getFRID().setRVER(updtFeat.getFRID().getRVER());

    //****** Modify attributes (ATTF, NATF) ******
    const std::map<unsigned short, FieldAttr> & attrUpdt = updtFeat.getAttribs();
    std::map<unsigned short, FieldAttr>::const_iterator upAtIt = attrUpdt.begin();
    for (; upAtIt != attrUpdt.end(); ++upAtIt)
    {
        if (isDeleteAttribute(upAtIt->second))
        {
            oldFeat.getAttribs().erase(upAtIt->first);
        }
        else
        {
            oldFeat.getAttribs()[upAtIt->first] = upAtIt->second;
        }
    }

    //****** Modify Feature-Feature-Relationships (FFPT) ******
    if (updtFeat.getFFPC().getFFUI() == 0)
    {
        if (updtFeat.getFfptVec().size() > 0) throw QString("%1 ERROR: FFPT found, but bo update instruction ").arg(updtFeat.getNameASCII());
    }
    else
    {
        std::vector < FieldFFPT > & oldFFPTs = oldFeat.getFfptVec();

        ulong startInd = updtFeat.getFFPC().getFFIX() -1; //rem: S-57 index starts with 1, but c-vector index with 0 !!!
        ulong number = updtFeat.getFFPC().getNFPT();
            
        //** insert **
        if (updtFeat.getFFPC().getFFUI() == 1) 
        {
            const std::vector < FieldFFPT > & updtFFPTs = updtFeat.getFfptVec();
            if (startInd > oldFFPTs.size() || number > updtFFPTs.size())
            {
                throw QString("%1 ERROR: Cannot insert FFPTs, vector index/size out of range!").arg(updtFeat.getNameASCII());
            }
            oldFFPTs.insert(oldFFPTs.begin() + startInd, updtFFPTs.begin(), updtFFPTs.begin() +number);
        }
        //** delete **
        else if (updtFeat.getFFPC().getFFUI() == 2) 
        {
            if (startInd >= oldFFPTs.size())
            {
                throw QString("%1 ERROR: Cannot delete FFPTs, start index out of range: %1 >= %2 ").arg(updtFeat.getNameASCII()).arg(startInd).arg(oldFFPTs.size());
            }
            if (startInd == 0 && number >= oldFFPTs.size())  //simple: delete ALL entries
            {
                oldFFPTs.clear();
            }
            else
            {
                oldFFPTs.erase(oldFFPTs.begin() + startInd, oldFFPTs.begin() + startInd +number);
            }
        }
        //** modify **
        else if (updtFeat.getFFPC().getFFUI() == 3) 
        {
            const std::vector < FieldFFPT > & updtFFPTs = updtFeat.getFfptVec();
            if (startInd + number > oldFFPTs.size())
            {
                throw QString("%1 ERROR: Cannot modify FFPTs, vector index/size out of range!").arg(updtFeat.getNameASCII());
            }
            if (number > updtFFPTs.size()) 
            {
                throw QString("%1 ERROR: Cannot modify FFPTs, too less FFPTs in update record!").arg(updtFeat.getNameASCII());
            }
            std::vector < FieldFFPT >::const_iterator updtIt = updtFFPTs.begin();
            std::vector < FieldFFPT >::iterator oldIt = oldFFPTs.begin() + startInd;
            for (uint cnt = 0; cnt < number; updtIt++, ++oldIt, ++cnt)
            {
                *oldIt = *updtIt;
            }
        }
    }

    //**** Modify Feature-Spatial-pointer (FSPT) ****
    if (updtFeat.getFSPC().getFSUI() == 0)
    {
        if (updtFeat.getFsptVec().size() > 0) throw QString("%1 ERROR: Update-FSPT found, but no update instruction!").arg(updtFeat.getNameASCII());
    }
    else
    {
        std::vector < FieldFSPT > & oldFSPTs = oldFeat.getFsptVec();
       
        ulong startInd = updtFeat.getFSPC().getFSIX() -1; //rem: S-57 index starts with 1, but c-vector index with 0 !!!
        ulong number = updtFeat.getFSPC().getNSPT();
         
        //** insert **
        if (updtFeat.getFSPC().getFSUI() == 1) 
        {
            const std::vector < FieldFSPT > & updtFSPTs = updtFeat.getFsptVec();
            if (startInd > oldFSPTs.size() || number > updtFSPTs.size())
            {
                throw QString("%1 ERROR: Cannot insert FSPTs, vector index/size out of range!").arg(updtFeat.getNameASCII());
            }
            oldFSPTs.insert(oldFSPTs.begin() + startInd, updtFSPTs.begin(), updtFSPTs.begin() +number);
        }
        //** delete **   
        else if (updtFeat.getFSPC().getFSUI() == 2) 
        {
            if (startInd >= oldFSPTs.size())
            {
                throw QString("%1 ERROR: Cannot delete FSPTs, start index out of range!").arg(updtFeat.getNameASCII());
            }
            if (startInd == 0 && number >= oldFSPTs.size())  //simple: delete ALL entries
            {
                oldFSPTs.clear();
            }
            else
            {
                oldFSPTs.erase(oldFSPTs.begin() + startInd, oldFSPTs.begin() + (startInd +number));
            }
        }
        //** modify **
        else if (updtFeat.getFSPC().getFSUI() == 3) 
        {
            const std::vector < FieldFSPT > & updtFSPTs = updtFeat.getFsptVec();
            if (startInd + number > oldFSPTs.size())
            {
                throw QString("%1 ERROR: Cannot modify FSPTs, vector index/size out of range!").arg(updtFeat.getNameASCII());
            }
            if (number > updtFSPTs.size()) 
            {
                throw QString("%1 ERROR: Cannot modify FSPTs, too less FFPTs in update record!").arg(updtFeat.getNameASCII());
            }
            unsigned int updtInd = 0, oldInd = startInd;
            for (; updtInd < number; ++updtInd, ++oldInd)
            {
                oldFSPTs[oldInd] = updtFSPTs[updtInd];
            }
        }
        else
        {
            throw QString("%1 ERROR: Unknown update intruction for FSPC!").arg(updtFeat.getNameASCII());
        }
    }
    reIssue->newFeatures.push_back(oldFeat.getFRID().getRCID());
}

//******************************************************************************
/// Apply one Spatial Update Record to dest Cell
/*!
* 
***************************************************************************** */
void S57Updater::applyUpdtSpatial(const SpatialS57 * updtSpat)
{
    //**** first: check if Spatial has to be deleted or inserted, only ****
    bool spatExists = reIssue->recordExists(updtSpat->getVRID().getRCNM(), updtSpat->getVRID().getRCID());
    if (updtSpat->getVRID().getRUIN() == 1) //insert Vector Record into ReIssue
    {
        if (spatExists) 
        {
            throw QString("%1 ERROR: Cannot insert Vector Record: Record already exists!").arg(updtSpat->getNameASCII());
        }
        reIssue->addSpatial(*updtSpat); 
    }
    else if (updtSpat->getVRID().getRUIN() == 2) //Delete Vector Record from ReIssue
    {
        if (!spatExists) 
        {
            throw QString("%1 ERROR: Cannot delete Vector Record: Record does not exists!").arg(updtSpat->getNameASCII());
        }
        reIssue->deleteRecordAndBackPointer(updtSpat->getVRID().getRCNM(), updtSpat->getVRID().getRCID());
    }
    else if (updtSpat->getVRID().getRUIN() == 3) //Modifiy FeVector Record in Reissue
    {
        if (!spatExists) 
        {
            throw QString("%1 ERROR: Cannot modify Vector Record: Record does not exists!").arg(updtSpat->getNameASCII());
        }
        modifySpatial(updtSpat);
    }
    else
    {
        throw QString("%1 ERROR: Invalid Vector Record update instruction (RUIN)!").arg(updtSpat->getNameASCII());
    }
}

//******************************************************************************
/// 
/*!
* 
***************************************************************************** */
void S57Updater::modifySpatial(const SpatialS57 * updtSpat)
{
    //** get the corresponding reissue-spatial **
    SpatialS57 * oldSpat = &reIssue->getSpatial(updtSpat->getVRID().getRCNM(), updtSpat->getVRID().getRCID());

    //** check and update the record-edition **
    if (oldSpat->getVRID().getRVER() + 1 != updtSpat->getVRID().getRVER())
    {
        throw QString("%1 ERROR: Cannot update Spatial: record-version not sequential!").arg(updtSpat->getNameASCII());
    }
    oldSpat->getVRID().setRVER(updtSpat->getVRID().getRVER());

    //** apply changes **
    modifySpatialCommons(oldSpat, updtSpat);

    if (updtSpat->getType() == Record8211::NodeUPDT)
    {
        applyUpdt2Node(*static_cast<NodeS57*>(oldSpat), *static_cast<const NodeS57_Updt*>(updtSpat));
    }
    else if (updtSpat->getType() == Record8211::SoundingUPDT)
    {
        applyUpdt2Soundg(*static_cast<SoundgS57*>(oldSpat), *static_cast<const SoundgS57_Updt*>(updtSpat));
    }
    else if (updtSpat->getType() == Record8211::EdgeUPDT)
    {
        applyUpdt2Edge(*static_cast<EdgeS57*>(oldSpat), *static_cast<const EdgeS57_Updt*>(updtSpat));
    }
}

//******************************************************************************
/// Modify fields common to all spatial types 
/*!
* Only Attributes are common to all Spatial, all other Fields must be modified by specialized methods
***************************************************************************** */
void S57Updater::modifySpatialCommons(SpatialS57 * oldSpat, const SpatialS57 * updtSpat)
{
    //****** Modify Spatial attributes (always lexlevel 0) ******
    const std::map < unsigned short, QString > & attrUpdt = updtSpat->getAttributes();
    std::map < unsigned short, QString >::const_iterator upAtIt = attrUpdt.begin();
    for (; upAtIt != attrUpdt.end(); ++upAtIt)
    {
        std::map < unsigned short, QString > & oldAttr = oldSpat->getAttributes();
        if (upAtIt->second.at(0) == ISO8211::DelChar)
        {
            oldAttr.erase(upAtIt->first);
        }
        else
        {
            oldAttr[upAtIt->first] = upAtIt->second;
        }
    }
}
  
//******************************************************************************
/// 
/*!
* 
***************************************************************************** */
void S57Updater::applyUpdt2Node(NodeS57 & orgNode, const NodeS57_Updt & updtNode)
{
    //** Nodes have exactly one vertex -> only "Modify" makes sense **
    if (updtNode.getSgcc().getCCUI() == 3)
    {
        orgNode.setLatLon(updtNode.getLat(), updtNode.getLon());
    }
    else if (updtNode.getSgcc().getCCUI() != 0)
    {
        throw QString("%1 ERROR: CCUI does not make sense for Node").arg(updtNode.getNameASCII());
    }
}

//******************************************************************************
/// 
/*!
* 
***************************************************************************** */
void S57Updater::applyUpdt2Soundg(SoundgS57 & orgSndg, const SoundgS57_Updt & updtSndg)
{
    if (updtSndg.getSgcc().getCCUI() == 0)
    {
        if (updtSndg.getSG3Dvec().size() > 0) throw QString("%1 ERROR: 3D coordinates in update Record, but no update intruction!").arg(updtSndg.getNameASCII());
    }
    else
    {
        std::vector < double > & orgCoords = orgSndg.getSG3Dvec();  //rem: triplets: Lat,Lon,Z,Lat,Lon,Z, ...
        unsigned int startInd = updtSndg.getSgcc().getCCIX() -1; //rem: S-57 index starts with 1, but c-vector index with 0 !!!
        unsigned int number = updtSndg.getSgcc().getCCNC();

        //**** case1: insert verteces ****
        if (updtSndg.getSgcc().getCCUI() == 1) 
        {
            const std::vector < double > & updtCoords = updtSndg.getSG3Dvec();
            if (startInd > orgCoords.size() /3 || number > updtCoords.size() /3)
            {
                throw QString("%1 ERROR: Cannot insert edge vertices, vector index/size out of range!").arg(updtSndg.getNameASCII());
            }
            orgCoords.insert(orgCoords.begin() + startInd *3, updtCoords.begin(), updtCoords.begin() +number*3);      
        }
        //**** case2: delete vertices ****
        else if (updtSndg.getSgcc().getCCUI() == 2) 
        {
            if (startInd >= orgCoords.size() /3)
            {
                throw QString("%1 ERROR: Cannot delete edge vertices, start index out of range!").arg(updtSndg.getNameASCII());
            }
            if (startInd == 0 && number >= orgCoords.size() /3)  //simple: delete ALL entries
            {
                orgCoords.clear();
            }
            else
            {
                orgCoords.erase(orgCoords.begin() + startInd *3, orgCoords.begin() + startInd *3 +number *3);
            }            
        }
        //**** case3: modify vertices ****
        else if (updtSndg.getSgcc().getCCUI() == 3) //modify
        {
            const std::vector < double > & updtCoords = updtSndg.getSG3Dvec();
            if (startInd*3 + number*3 > orgCoords.size())
            {
                throw QString("%1 ERROR: Cannot modify edge vertices, vector index/size out of range!").arg(updtSndg.getNameASCII());
            }
            if (number *3 > updtCoords.size()) 
            {
                throw QString("%1 ERROR: Cannot modify edge vertices, too less vertices in update record!").arg(updtSndg.getNameASCII());
            }
            unsigned int updtIndex = 0;
            unsigned int orgIndex = startInd*3;
            for (; updtIndex < number *3; updtIndex++, ++orgIndex)
            {
                orgCoords[orgIndex] = updtCoords[updtIndex];
            }
        }
        else 
        {
            throw QString("%1 ERROR: CCUI - invalid value for update instruction").arg(updtSndg.getNameASCII());
        }
    }
}

//******************************************************************************
/// 
/*!
* 
***************************************************************************** */
void S57Updater::applyUpdt2Edge(EdgeS57 & orgEdge, const EdgeS57_Updt & updtEdge)
{
    if (updtEdge.getSgcc().getCCUI() == 0)
    {
        if (updtEdge.getSG2Dvec().size() > 0) throw QString("%1 ERROR: coordinates in edge-update found, but no update intruction!").arg(updtEdge.getNameASCII());
    }
    else
    {
        std::vector < double > & orgCoords = orgEdge.getSG2Dvec();
        unsigned int startInd = updtEdge.getSgcc().getCCIX() -1; //rem: S-57 index starts with 1, but c-vector index with 0 !!!
        unsigned int number = updtEdge.getSgcc().getCCNC();

        if (updtEdge.getSgcc().getCCUI() == 1) //insert
        {
            const std::vector < double > & updtCoords = updtEdge.getSG2Dvec();
            if (startInd > orgCoords.size() /2 || number > updtCoords.size() /2)
            {
                throw QString("%1 ERROR: Cannot insert edge vertices, vector index/size out of range!").arg(updtEdge.getNameASCII());
            }
            orgCoords.insert(orgCoords.begin() + startInd *2, updtCoords.begin(), updtCoords.begin() +number*2);            
        }
        //**** case2: delete vertices ****
        else if (updtEdge.getSgcc().getCCUI() == 2)  
        {
            if (startInd >= orgCoords.size() /2)
            {
                throw QString("%1 ERROR: Cannot delete edge vertices, start index out of range!").arg(updtEdge.getNameASCII());
            }
            if (startInd == 0 && number >= orgCoords.size() /2)  //simple: delete ALL entries
            {
                orgCoords.clear();
            }
            else
            {
                orgCoords.erase(orgCoords.begin() + startInd *2, orgCoords.begin() + startInd *2 +number *2);
            }
        }
        //**** case3: modify vertices ****
        else if (updtEdge.getSgcc().getCCUI() == 3) 
        {
            const std::vector < double > & updtCoords = updtEdge.getSG2Dvec();
            if (startInd*2 + number*2 > orgCoords.size())
            {
                throw QString("ERROR: Cannot modify edge vertices, vector index/size out of range!").arg(updtEdge.getNameASCII());
            }
            if (number *2 > updtCoords.size()) 
            {
                throw QString("ERROR: Cannot modify edge vertices, too less vertices in update record!").arg(updtEdge.getNameASCII());
            }
            unsigned int updtIndex = 0;
            unsigned int orgIndex = startInd*2;
            for (; updtIndex < number *2; updtIndex++, ++orgIndex)
            {
                orgCoords[orgIndex] = updtCoords[updtIndex];
            }
        }
        else
        {
            throw QString("ERROR: Cannot update edge vertices: CCUI - invalid value").arg(updtEdge.getNameASCII());
        }
    }
    reIssue->newEdges.push_back(orgEdge.getVRID().getRCID());
}
