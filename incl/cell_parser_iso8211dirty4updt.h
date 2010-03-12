#ifndef CELL_PARSER_ISO8211_DIRTY4Update
#define CELL_PARSER_ISO8211_DIRTY4Update

#include "cell_parser_iso8211dirty.h"
#include "cell_s57.h"

namespace Enc
{
class CellS57_Update;

//*****************************************************************************
/// Simplified Class to parse a complete ISO-8211 ENC update file 
/*!
* It can only parse update files ("ER") 
****************************************************************************** */    
class CellParser8211Dirty4Updt : public CellParser8211Dirty
{
public:
    CellParser8211Dirty4Updt(CellS57_Update * cell =0, unsigned long parseOptions =CellParser8211::ParseAll);
    CellParser8211Dirty4Updt(double factorXY, double factorZ, unsigned long parseOptions =CellParser8211::ParseAll);
    virtual ~CellParser8211Dirty4Updt();
    virtual void setCell(CellS57_Header * cell);
    virtual void parseS57Cell(QString cellName);

protected:
    //** Feature Record Field Parser **
    virtual void parseFeatureRecord(Iso8211fieldIterator & fieldIt);

    //** Vector Record Field Parser **
    virtual void parseVectorRecord(Iso8211fieldIterator & fieldIt);
    void parseVectorUpdtRecord(Iso8211fieldIterator & fieldIt, SpatialS57 * updtSpatial);
    void parseVectorUpdtFields(Iso8211fieldIterator & fieldIt, SpatialS57_Updt &);

    //**** Data ****
    CellS57_Update * cellS57;
};

}

#endif