#ifndef CELL_S57_CHECK_H
#define CELL_S57_CHECK_H

#include "cell_s57_base.h"


namespace Enc
{

//******************************************************************************
/// Do basic checks to find out if a ENC is corrupted
/*!
* Check S-57 Base-Cell without considering the Product Type
* - All References and Back-References(Pointer) are checked:
*   Feat-Feat, Feat-Spatial, Spatial-Spatial
***************************************************************************** */
class CellCheckS57
{
public:
    CellCheckS57(const CellS57_Base * cell2check =0) : cell(cell2check) {}

    void setCell(const CellS57_Base * cell2check) {cell = cell2check;}
    const CheckResult & check();

protected:

    void checkSpatial2FeatureRelation(const SpatialS57 & spat);

    const CellS57_Base * cell;
    CheckResult checkRes;
};
}
#endif