#include "cell_writer.h"

#include <QtCore/QFile>

using namespace Enc;

void CellWriter::setCell(const CellS57_Base * cell)
{
    cellS57 = cell;
}
 
void CellWriter::writeS57Cell(QString cellName)
{
    if (!cellS57) throw QString("ERROR: Cannot write S-57 Cell %1 No Cell Pointer!").arg(cellName);
    QFile iso8211file(cellName);
    if (!iso8211file.open(QIODevice::WriteOnly)) throw QString("ERROR: cannot open S-57 File %1 for writing").arg(cellName);
    writeS57Cell(&iso8211file);  
    iso8211file.close();
}