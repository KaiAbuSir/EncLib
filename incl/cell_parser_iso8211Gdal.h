#ifndef CELL_PARSER_ISO8211_GDAL
#define CELL_PARSER_ISO8211_GDAL

#include "cell_parser_iso8211.h"
#include "iso8211.h"
#include <QtCore/QString>

namespace Enc
{
class CellS57_Base;

//*****************************************************************************
/// Parse a complete ISO-8211 ENC update file, taking care of ALL of the ISO8211 standard
/*!
* NOT YET FINISHED !!! WORK IN PROGRESS
* 
****************************************************************************** */    
class CellParser8211GDAL : public CellParser8211
{
public:

    CellParser8211GDAL(CellS57_Base * cell = 0);
    virtual void setCell(CellS57_Base * cell);

private:
    //**** Methods ****
    virtual void parseS57CellIntern(QString cellName);

    void ParseHeaderRecord(DDFRecord & rec);
    void ParseFeatureRecord(DDFRecord & rec);

    //**** Data ****
    CellS57_Base * cellS57;

    DDFModule  oModule;  //DDR (and rest of file)

};

}

#endif