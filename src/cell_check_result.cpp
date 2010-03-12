#include "cell_check_result.h"

#include <QtCore/QString>

using namespace Enc;
using namespace ISO8211;


void CheckResult::makeMessage(QStringList & errMsg) const
{
    unsigned int i=0;
    if (!brokenFeat2Spatial.empty())
    {
        errMsg += QString("%1 Features pointing to non existing Spatials:").arg(brokenFeat2Spatial.size());
        for (i=0; i <brokenFeat2Spatial.size(); ++i) //std::vector< FeatSpatPair >
        {
            errMsg += QString("Feature %1 Spatial: %2").arg(makeRecNameASCII(RCNM_FE, brokenFeat2Spatial[i].first)).arg(makeRecNameASCII(brokenFeat2Spatial[i].second)); 
        }
    }
    if (!brokenSpatial2Feat.empty())
    {
        errMsg += QString("%1 Spatials pointing to non existing Features:").arg(brokenSpatial2Feat.size());
        for (i=0; i < brokenSpatial2Feat.size(); ++i)
        {
            errMsg += QString("Spatial: %1 broken Feature: %2").arg(makeRecNameASCII(brokenSpatial2Feat[i].first)).arg(makeRecNameASCII(RCNM_FE, brokenSpatial2Feat[i].second));
        }
    }
    if (!brokenFeat2Feat.empty())
    {
        errMsg += QString("%1 feature to feature relation errros (internal calculation error)").arg(brokenFeat2Feat.size());
        for (i=0; i < brokenFeat2Feat.size(); ++i)
        {
            errMsg += QString("Existing Feature: %1 Broken Feature %2").arg(makeRecNameASCII(RCNM_FE, brokenFeat2Feat[i].first)).arg(makeRecNameASCII(RCNM_FE, brokenFeat2Feat[i].second));
        }
    }
    if (!brokenFeat2FeatBack.empty())
    {
        errMsg += QString("%1 feature to feature backward relation errros (internal calculation error)").arg(brokenFeat2FeatBack.size());
        for (i=0; i <brokenFeat2Spatial.size(); ++i) //std::vector< FeatSpatPair >
        {
            errMsg += QString("Feature %1 Broken Feature: %2").arg(ISO8211::makeRecNameASCII(RCNM_FE, brokenFeat2FeatBack[i].first)).arg(ISO8211::makeRecNameASCII(RCNM_FE, brokenFeat2FeatBack[i].second)); 
        }
    }
    if (!orphanFeatures.empty())
    {
        errMsg += QString("%1 Orphaned Geo-Features (Point,Line or Area Geometry): ");
        for (i=0; i < orphanFeatures.size(); ++i)
        {
            errMsg += makeRecNameASCII(RCNM_FE, orphanFeatures[i]);
        }
    }
    if (!orphanSpatials.empty())
    {
        errMsg += QString("%1 Orphaned Spatials:").arg(orphanSpatials.size());
        for (i=0; i < orphanSpatials.size(); ++i)
        {
            errMsg += makeRecNameASCII(orphanSpatials[i]);
        }
    }
    if (!brokenEdge2Node.empty())
    {
        errMsg += QString("%1 Broken Edge to Bounding Node Pointer:").arg(brokenEdge2Node.size());
        for (i=0; i < brokenEdge2Node.size(); ++i)
        {
            errMsg += QString("Edge: %1 Broken Node: %2").arg(makeRecNameASCII(RCNM_VE, brokenEdge2Node[i].first)).arg(makeRecNameASCII(RCNM_VC, brokenEdge2Node[i].second));
        }
    }
    if (!edgesMissingBNodes.empty())
    {
        errMsg += QString("%1 Edges missing Bounding Nodes:").arg(edgesMissingBNodes.size());
        for (i=0; i < edgesMissingBNodes.size(); ++i)
        {
            errMsg += makeRecNameASCII(RCNM_VE, edgesMissingBNodes[i]);
        }
    }
    if (!bNodesMissingEdges.empty())
    {
        errMsg += QString("%1 Bounding Nodes not connected to Edges:").arg(bNodesMissingEdges.size());
        for (i=0; i < bNodesMissingEdges.size(); ++i)
        {
            errMsg += makeRecNameASCII(RCNM_VC, bNodesMissingEdges[i]);
        }
    }
    if (!brokenBNodes2Edge.empty())
    {
        errMsg += QString("%1 Bounding Nodes with broken Edge Pointer (internal error):").arg(brokenBNodes2Edge.size());
        for (i=0; i < brokenBNodes2Edge.size(); ++i)
        {
            errMsg += QString("Bounding Node: %1 Broken Edge Pointer: %2").arg(makeRecNameASCII(RCNM_VC, brokenBNodes2Edge[i].first)).arg(makeRecNameASCII(RCNM_VE, brokenBNodes2Edge[i].second));
        }
    }
    if (!featMissingFOID.empty())
    {
        errMsg += QString("%1 Features without valid FOID").arg(featMissingFOID.size());
        for (i=0; i < featMissingFOID.size(); ++i)
        {
            errMsg += makeRecNameASCII(RCNM_FE, featMissingFOID[i]);
        }
    }
    //**** Non-Error messages: ****
    if (!outsidePointer.empty())
    {
        errMsg += QString("%1 FOIDs pointing to Features in other Cells: ").arg(outsidePointer.size());
        for (i=0; i < outsidePointer.size(); ++i)
        {
            errMsg += makeRecNameASCII(RCNM_FE, outsidePointer[i].first) +" has OutsideCellPointer: " +ISO8211::makeLongNAMeASCII(outsidePointer[i].second);
        }
    }
}