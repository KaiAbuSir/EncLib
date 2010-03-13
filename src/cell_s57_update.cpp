//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_s57_update.h"

#include <algorithm>

#include "cell_parser_iso8211dirty4updt.h"
#include "cell_writer_iso8211dirty.h"

using namespace Enc;

CellS57_Update::CellS57_Update() : CellS57_Header()
{
}
CellS57_Update::CellS57_Update(const FieldDSPM & baseDSPM) : CellS57_Header(baseDSPM)
{
}

CellS57_Update::~CellS57_Update()
{
    clear();
}

//******************************************************************************
/// Clear All Container, free all Memory - Cell becomes an empty cell again
//**************************************************************************** */
void CellS57_Update::clear()
{
    CellS57_Header::clear();

    for (std::vector <FeatureS57_Updt *>::iterator it = features.begin(); it != features.end(); ++it) delete *it;
    features.clear();
    for (std::vector <SpatialS57 *>::iterator sIt = spatials.begin(); sIt != spatials.end(); ++sIt) delete *sIt;
    spatials.clear();
}

void CellS57_Update::clearExceptDSPM()
{
    CellS57_Header::clearExceptDSPM();

    for (std::vector <FeatureS57_Updt *>::iterator it = features.begin(); it != features.end(); ++it) delete *it;
    features.clear();
    for (std::vector <SpatialS57 *>::iterator sIt = spatials.begin(); sIt != spatials.end(); ++sIt) delete *sIt;
    spatials.clear();
}

//******************************************************************************
/// Conveniance Method: Fill update cell by parsing a update-File
/*! File is paresed completely, except the DDR
 **************************************************************************** */
void CellS57_Update::parseISO8211(QString fileNamePath)
{
    CellParser8211Dirty4Updt s57parser(this, CellParser8211::ParseAll);
    s57parser.parseS57Cell(fileNamePath);
}
