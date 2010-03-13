//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_s57.h"

#include <algorithm>

#include "cell_parser_iso8211dirty4base.h"
#include "cell_parser_iso8211dirty4updt.h"
#include "cell_writer_iso8211dirty.h"

using namespace Enc;

CellS57_Header::CellS57_Header() : DDR(0), DDRlen(0)
{}

CellS57_Header::CellS57_Header(const FieldDSPM & baseDSPM) : dspm(baseDSPM), DDR(0), DDRlen(0)
{}


CellS57_Header::~CellS57_Header() 
{
    delete [] DDR;
}

void CellS57_Header::clear()
{
    dsid.clear();
    dssi.clear();
    dspm.clear();
    delete [] DDR;
    DDRlen = 0;
}

void CellS57_Header::clearExceptDSPM()
{
    dsid.clear();
    dssi.clear();
    delete [] DDR;
    DDRlen = 0;
}

void CellS57_Header::putDDR(const char * ddrPtr, int len)
{
    if (DDR && DDRlen != len)
    {
        delete [] DDR;
        DDR = new char[len];
    }
    DDRlen = len;
    memcpy(DDR, ddrPtr, DDRlen);
}
