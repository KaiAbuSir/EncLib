#ifndef CELL_PARSER_ISO8211_DIRTY4BASE
#define CELL_PARSER_ISO8211_DIRTY4BASE


#include "cell_parser_iso8211dirty.h"

namespace Enc
{
class CellS57_Base;

//*****************************************************************************
/// Simplified Class to parse a complete ISO-8211 fiele containing a S-57 BASE ENC
/*!
* Since the S-57 BASE ENCs only use a small subset of ISO8211, this class is simplified,
* but cannot parse other files than S-57 BASE Files
****************************************************************************** */
class CellParser8211Dirty4Base : public CellParser8211Dirty
{
public:
    CellParser8211Dirty4Base(CellS57_Base * cell =0, unsigned long parseOptions = ParseAll);
    virtual ~CellParser8211Dirty4Base();
    virtual void setCell(CellS57_Header * cell);
    virtual void parseS57Cell(QString cellName);

protected:
    //** Feature Record Field Parser **
    virtual void parseFeatureRecord(Iso8211fieldIterator & fieldIt);

    //** Vector Record Field Parser **
    virtual void parseVectorRecord(Iso8211fieldIterator & fieldIt);

    //**** Data ****
    CellS57_Base * cellS57;
};

}

#endif