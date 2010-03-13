//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include <QtCore/QFile>

#include "cell_parser_iso8211.h"
#include "iso8211_simple.h"

using namespace Enc;

CellParser8211::CellParser8211(unsigned long parseOptions) : parseOpts(parseOptions)
{}

CellParser8211::~CellParser8211()
{}

