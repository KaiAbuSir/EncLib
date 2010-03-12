#ifndef S57_UPDATER_H
#define S57_UPDATER_H

#include "cell_s57_base.h"
#include "cell_s57_update.h"

namespace Enc
{

//******************************************************************************
/// Applies S-57 Update Cells to Base Cells to create ReIssue Cells
/*!
***************************************************************************** */
class S57Updater 
{
public:
    S57Updater();
    ~S57Updater();
    void setCell(CellS57_Base * baseCellS57);
    void applyUpdate(const CellS57_Update *);

private:

    void applyUpdtFeature(const FeatureS57_Updt & updtFeat);
    void applyUpdtSpatial(const SpatialS57 * spat);

    void modifyFeature(FeatureS57 & oldFeat, const FeatureS57_Updt & updtFeat);

    void modifySpatial(const SpatialS57 * updtSpat);
    void modifySpatialCommons(SpatialS57 * oldSpat, const SpatialS57 * updtSpat);

    void applyUpdt2Node(NodeS57 & orgNode, const NodeS57_Updt & updtNode);
    void applyUpdt2Soundg(SoundgS57 & orgSndg, const SoundgS57_Updt & updtSndg);
    void applyUpdt2Edge(EdgeS57 & orgEdge, const EdgeS57_Updt & updtEdge);

    bool isDeleteAttribute(const FieldAttr & attr);

    CellS57_Base * reIssue;  //Cell to which updates will be applied
};

inline bool S57Updater::isDeleteAttribute(const FieldAttr & attr)
{
    if (!attr.getNat() || reIssue->getDssi().getNALL() == 1)
    {
        return attr.getValue().at(0) == ISO8211::DelChar;
    }
    else 
    {
        return attr.getValue().at(0) == ISO8211::DelChar2;
    }
}

}
#endif