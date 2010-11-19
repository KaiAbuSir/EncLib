
//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_check_s57.h"

using namespace Enc;

//*****************************************************************************
/// Check the record contents without using the product spec
/*!
* The checks are not ENC specific, only what S-57 specific
****************************************************************************** */    
const CheckResult &  CellCheckS57::check()
{
    checkRes.clear();

    const std::map< unsigned long, FeatureS57 *> & features = cell->getFeatures();
    std::map< unsigned long, FeatureS57 *>::const_iterator feIt = features.begin();  
    for(; feIt != features.end(); ++feIt)
    {
        //**** check feature-feature, if possible ****
        const FeatureS57 * feat = feIt->second;
        const std::vector < FieldFFPT > & FFPTvec = feat->getFfptVec();
        for (uint ffInd = 0; ffInd < FFPTvec.size(); ++ffInd)
        {
            unsigned long otherRcid = cell->getRcid4Foid(FFPTvec[ffInd].getLNAM()); //may be 0 if feature is not in same cell
            if (otherRcid && !cell->recordExists(ISO8211::RCNM_FE, otherRcid))
            {
                checkRes.brokenFeat2Feat.push_back(RCIDPair(feat->getFRID().getRCID(),otherRcid));
            }
            if (!otherRcid)
            {
                checkRes.outsidePointer.push_back(RcidFoidPair(feat->getFRID().getRCID(), FFPTvec[ffInd].getLNAM()));
            }
        }
        //**** check feat-feat-back-pointer ****
        const std::vector< unsigned long > & ffptBackVec = feat->getFfptBackVec();
        for (uint bI = 0; bI < ffptBackVec.size(); ++bI)
        {
            if (!cell->recordExists(ISO8211::RCNM_FE , ffptBackVec[bI]))
            {
                checkRes.brokenFeat2FeatBack.push_back(RCIDPair(feat->getFRID().getRCID(), ffptBackVec[bI]));
            }
        }

        //**** check feature->Spatial ****
        const std::vector < FieldFSPT > & FSPTvec = feat->getFsptVec();
        for (uint fsInd = 0; fsInd < FSPTvec.size(); ++fsInd)
        {
            if (!cell->recordExists(FSPTvec[fsInd].getOtherRCNM() ,FSPTvec[fsInd].getOtherRCID()))
            {
                checkRes.brokenFeat2Spatial.push_back(std::pair<unsigned long, RecNAME>(feat->getFRID().getRCID(), FSPTvec[fsInd].getOtherRecName()));
            }
        }
        if (feat->getFRID().getPRIM() != 255 && FSPTvec.empty())
        {
            checkRes.orphanFeatures.push_back(feat->getFRID().getRCID());
        }
    }

    const std::map< unsigned long, NodeS57 *> & iNodes = cell->getINodes();
    std::map< unsigned long, NodeS57 *>::const_iterator inIt = iNodes.begin();      
    for (; inIt != iNodes.end(); ++inIt)
    {
        checkSpatial2FeatureRelation(*inIt->second);
    }

    const std::map< unsigned long, BoundNodeS57 *> & bNodes = cell->getBNodes();
    std::map< unsigned long, BoundNodeS57 *>::const_iterator bnIt = bNodes.begin(); 
    for (; bnIt != bNodes.end(); ++bnIt)
    {
        checkSpatial2FeatureRelation(*bnIt->second);
        //** check if edge back-pointers are OK **
        const std::vector <unsigned long> edges = bnIt->second->getEdges();
        if (edges.empty())
        {
            checkRes.bNodesMissingEdges.push_back(bnIt->second->getVRID().getRCID());
        }
        for (uint eI = 0; eI < edges.size(); ++ eI)
        {
            if(!cell->recordExists(ISO8211::RCNM_VE, edges[eI]))
            {
                checkRes.brokenBNodes2Edge.push_back(RCIDPair(bnIt->second->getVRID().getRCID(), edges[eI]));
            }
        }
    }
    
    const std::map< unsigned long, SoundgS57 *> & soundings = cell->getSoundingds();
    std::map< unsigned long, SoundgS57 *>::const_iterator sdIt = soundings.begin(); 
    for (;sdIt != soundings.end(); ++sdIt)
    {
        checkSpatial2FeatureRelation(*sdIt->second);
    }

    const std::map< unsigned long, EdgeS57 *> & edges = cell->getEdges();
    std::map< unsigned long, EdgeS57 *>::const_iterator edIt = edges.begin();
    for (;edIt != edges.end(); ++edIt)
    {
        const EdgeS57 * edge = edIt->second;
        checkSpatial2FeatureRelation(*edge);
        //** check if Bounding Nodes are OK **
        if (edge->getStartNodeRecId() == 0 || edge->getEndNodeRecId() == 0)
        {
            checkRes.edgesMissingBNodes.push_back(edge->getVRID().getRCID());
        }
        if (edge->getStartNodeRecId() && !cell->recordExists(ISO8211::RCNM_VC , edge->getStartNodeRecId()))
        {
            checkRes.brokenEdge2Node.push_back(RCIDPair(edge->getVRID().getRCID(), edge->getStartNodeRecId()));
        }
        if (edge->getEndNodeRecId() && !cell->recordExists(ISO8211::RCNM_VC , edge->getEndNodeRecId()))
        {
            checkRes.brokenEdge2Node.push_back(RCIDPair(edge->getVRID().getRCID(), edge->getEndNodeRecId()));
        }
    }
    return checkRes;
}

//*****************************************************************************
/// Check the Spatial to Feature Pointer
/*!
* 
****************************************************************************** */    
void CellCheckS57::checkSpatial2FeatureRelation(const SpatialS57 & spat)
{
   const std::vector <unsigned long> & relFeatures = spat.getFeatures();
   for (uint fI = 0; fI < relFeatures.size(); ++fI)
   {
        if (!cell->recordExists(ISO8211::RCNM_FE , relFeatures[fI]))
        {
             checkRes.brokenSpatial2Feat.push_back(SpatFeatPair(spat.getNameBIN(), relFeatures[fI]));
        }
   }
   //** Isolated nodes (incl. soundings) and Edges MUST have a relation to a feature **
   if ((spat.getVRID().getRCNM() == ISO8211::RCNM_VI || spat.getVRID().getRCNM() == ISO8211::RCNM_VE) && relFeatures.empty())
   {
        checkRes.orphanSpatials.push_back(spat.getNameBIN());
   }
}
