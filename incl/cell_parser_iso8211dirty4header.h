#ifndef CELL_PARSER_ISO8211_DIRTY4HEADER
#define CELL_PARSER_ISO8211_DIRTY4HEADER

#include "cell_parser_iso8211dirty.h"

namespace Enc
{

//*****************************************************************************
/// Simplified Class to parse the HEADER of a ISO-8211 file containing a S-57 BASE ENC
/*!
* It only parses the 3 Fields contained in the first and second Data-Record,
* but does not parse vector- or feature-records -> fast, when only header-data is needed
****************************************************************************** */
class CellParser8211Dirty4Header : public CellParser8211Dirty
{
public:
    CellParser8211Dirty4Header(CellS57_Header * cell =0, unsigned long parseOptions =0);
    virtual ~CellParser8211Dirty4Header();
    virtual void setCell(CellS57_Header * cell);
    virtual void parseS57Cell(QString cellName);

protected:

    virtual void parseFeatureRecord(Iso8211fieldIterator & fieldIt) {throw QString("Internal Error: Trying to parse Feature in Header of cell: %1!").arg(cellS57 ? cellS57->getDsid().getDSNM():"<No Cell>");}
    virtual void parseVectorRecord(Iso8211fieldIterator & fieldIt) {throw QString("Internal Error: Trying to parse Spatial in Header of cell: %1!").arg(cellS57 ? cellS57->getDsid().getDSNM():"<No Cell>");}

    //**** Data ****
    CellS57_Header * cellS57;
};

}

#endif