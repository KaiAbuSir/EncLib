#ifndef CELL_CHECK_RESULT_H
#define CELL_CHECK_RESULT_H

#include <QtCore/QStringList>

//#include <pair>
#include <vector>

#include "iso8211_simple.h"

namespace Enc
{
class CellParser8211;
class CellParser8211Dirty;
class CellWriter;
class CellWriter8211Dirty;
class CellParser8211Dirty4Base;
class S57Updater;

using namespace ISO8211;

//******************************************************************************
/// Struct counting errors when checking cells
/*!
* 
***************************************************************************** */
typedef std::pair<unsigned long, unsigned long> RCIDPair;
typedef std::pair<unsigned long, RecNAME> FeatSpatPair;
typedef std::pair<RecNAME, unsigned long> SpatFeatPair;
typedef std::pair<unsigned long, LongNAMe>RcidFoidPair;
struct CheckResult
{
    std::vector< FeatSpatPair >  brokenFeat2Spatial;  //features pointing to non existing spatials
    std::vector< SpatFeatPair >  brokenSpatial2Feat;  //spatial pointing to non existing Features (back-pointer error)
    std::vector< RCIDPair >      brokenFeat2Feat;     //feat-feat relation errro (internal calculation error)
    std::vector< RCIDPair >      brokenFeat2FeatBack; //feat-feat backward rel error (back-pointer error)
    std::vector< unsigned long > orphanFeatures;      //Features that should have (Point,Line or Area) Geometry, but have not
    std::vector< RecNAME >       orphanSpatials;      //Spatials not used by any feature
    std::vector< RCIDPair >      brokenEdge2Node;     //edges where Bounding node is not found in cell, pair1=edge-rcid, pair2= node-rcid(invalid)
    std::vector< unsigned long > edgesMissingBNodes;  //edges without bounding nodes at all
    std::vector< unsigned long > bNodesMissingEdges;  //Bounding nodes without edges
    std::vector< RCIDPair >      brokenBNodes2Edge;   //Node 2 Edge Pointer broken (back-pointer-error) pair1=node-rcid, pair2= edge-rcid(invalid)
    std::vector< unsigned long > featMissingFOID;     //Features without valid FOID;
    //** Not and Error -> Not counted in "errCnt", but suspicious: **
    std::vector< RcidFoidPair >  outsidePointer;      //Features that contain FOIDs pointing to features in other cells: pair1=Feat that holds the FOISs(=pair2)

    void clear() 
    {
        brokenFeat2Spatial.clear(); brokenSpatial2Feat.clear(); brokenFeat2Feat.clear(); brokenFeat2FeatBack.clear();  orphanFeatures.clear(); orphanSpatials.clear(); 
        brokenEdge2Node.clear(); edgesMissingBNodes.clear(); bNodesMissingEdges.clear(); brokenBNodes2Edge.clear(); featMissingFOID.clear();
        outsidePointer.clear();
    }
    int errCnt() const 
    {
        return brokenFeat2Spatial.size()+ brokenSpatial2Feat.size()+ brokenFeat2Feat.size()+ brokenFeat2FeatBack.size()+ orphanFeatures.size()+ orphanSpatials.size()+ 
               brokenEdge2Node.size()+ edgesMissingBNodes.size()+ bNodesMissingEdges.size()+ brokenBNodes2Edge.size()+ featMissingFOID.size();
    }
    void makeMessage(QStringList & errMsg) const;
};
}
#endif
