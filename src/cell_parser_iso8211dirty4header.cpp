//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include <QtCore/QFile>

#include "cell_parser_iso8211dirty4header.h"
#include "cell_records.h"
#include "iso8211_simple.h"

using namespace Enc;

CellParser8211Dirty4Header::CellParser8211Dirty4Header(CellS57_Header * cell, unsigned long parseOptions) : CellParser8211Dirty(parseOptions), cellS57(cell)
{}

CellParser8211Dirty4Header::~CellParser8211Dirty4Header()
{}


void CellParser8211Dirty4Header::setCell(CellS57_Header * cell)
{
   if (!cell)
    {
        cellS57 = NULL;
        return;
    }
    cellS57 = cell;
}

//*****************************************************************************
/// Parse the Header of an S-57 ISO8211 Base or Update Cell
/*!
* Only the first 4096 Bytes are Read to make parsing faster!
****************************************************************************** */
void CellParser8211Dirty4Header::parseS57Cell(QString cellName)
{
    parseInit(cellName, cellS57, 4096);
    parseS57CellIntern(cellS57);
}