#ifndef CELL_PARSER_ISO8211
#define CELL_PARSER_ISO8211

#include "iso8211_simple.h"

#include <QtCore/QString>

namespace Enc
{
class CellS57_Header;

class CellParser8211
{
public:

    enum ParseOptions{StoreDDR = 2, PrsVecRec = 4, PrsFeatRec = 16, FeatTextOnly = 64};
    const static int ParseAll = PrsVecRec | PrsFeatRec;
    const static int FullOptions = StoreDDR | ParseAll;
    
    CellParser8211(unsigned long parseOptions);
    virtual ~CellParser8211();
    virtual void setCell(CellS57_Header * cell) =0;
    void setOptions(unsigned long parseOptions) {parseOpts = parseOptions;}
    virtual void parseS57Cell(QString cellName) =0;

protected:

    unsigned long parseOpts;
};

}

#endif

