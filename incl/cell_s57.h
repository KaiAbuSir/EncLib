#ifndef CELL_S57
#define CELL_S57

#include "cell_records.h"

#include <QtCore/QString>

#include <vector>

namespace Enc
{
class CellParser8211;
class CellParser8211Dirty;
class CellWriter;
class CellWriter8211Dirty;
class CellParser8211Dirty4Base;
class CellParser8211Dirty4Updt;

//******************************************************************************
/// Class containig the data (records) common to all S57 Update AND Base Cells
/*!
* Also usefull if only the cellHeader is needed
***************************************************************************** */
class CellS57_Header
{
public:

    enum CellS57Class{HeaderOnly = 1, BaseCellClass = 2, UpdtCellClass = 3};

    CellS57_Header();
    CellS57_Header(const FieldDSPM & baseDSPM);
    virtual ~CellS57_Header();
    virtual CellS57Class getClass() const {return HeaderOnly;} //usefull for casting
    virtual void clear();
    virtual void clearExceptDSPM();

    void putDDR(const char * ddrPtr, int len);

    const FieldDSID & getDsid() const {return dsid;}
    const FieldDSSI & getDssi() const {return dssi;}
    const FieldDSPM & getDspm() const {return dspm;}
    
    FieldDSID & getDsid() {return dsid;}
    FieldDSSI & getDssi() {return dssi;}
    FieldDSPM & getDspm() {return dspm;}

    virtual bool isUpdateCell() const {return dsid.getEXPP() == 2;}

protected:

    //**** Header Data ****
    FieldDSID dsid;
    FieldDSSI dssi;
    FieldDSPM dspm;  //will be empty in case of UpdateCell

    //DDR record (including Leader) - unimportant - only needed if someone wants to read cell and then write original DDR when writing cell 
    //or if the DDR is non Standard (but that means: not a S-57 cell)
    char * DDR; 
    int DDRlen; //length of DDR
};

}

#endif