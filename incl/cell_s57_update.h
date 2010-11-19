#ifndef CELL_S57_UPDATE
#define CELL_S57_UPDATE

#include "cell_s57.h"

namespace Enc
{
class CellParser8211;
class CellParser8211Dirty;
class CellWriter;
class CellWriter8211Dirty;
class CellParser8211Dirty4Updt;

//******************************************************************************
/// Class containig the data (records) of a S-57 UPDATE cell 
/*!
* Does not contains maps, "backward"-pointer ... because performance is not that important for updates
***************************************************************************** */
class CellS57_Update : public CellS57_Header
{
friend class CellParser8211;
friend class CellParser8211Dirty;
friend class CellParser8211Dirty4Updt;

public:
    CellS57_Update(const FieldDSPM & baseDSPM);
    CellS57_Update();
    ~CellS57_Update();
    virtual CellS57Class getClass() const {return UpdtCellClass;} //usefull for casting

    virtual void clear();
    void clearExceptDSPM();
    virtual bool isUpdateCell() const {return true;}

    void parseISO8211(QString fileNamePath);

    const std::vector< FeatureS57_Updt *> &  getFeatures()  const {return features;}  
    const std::vector< SpatialS57 *> &     getSpatials()    const {return spatials;}       
 
protected:
    std::vector <FeatureS57_Updt *> features;
    std::vector <SpatialS57 *> spatials;
};
}
#endif
