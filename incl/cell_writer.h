#ifndef CELL_WRITER
#define CELL_WRITER

#include "iso8211_simple.h"

#include <QtCore/QString>

namespace Enc
{
class CellS57_Base;

//******************************************************************************
/// Interface for Classes writing Base-Cell to ISO8211 Files
/*!
***************************************************************************** */
class CellWriter
{
public:

    CellWriter(const CellS57_Base * cell=0) : cellS57(cell) {}
    virtual ~CellWriter() {}
    void setCell(const CellS57_Base * cell);
    void writeS57Cell(QString cellName);
    virtual void writeS57Cell(QIODevice * cellDev) = 0;

protected:

    const CellS57_Base * cellS57;
};

}

#endif

