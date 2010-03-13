//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_parser_iso8211Gdal.h"
#include "cell_records.h"
#include "cell_s57_base.h"
//Gdal includes:
#include "iso8211.h"
#include "cpl_vsi.h"


using namespace Enc;

CellParser8211GDAL::CellParser8211GDAL(CellS57_Base * cell) : CellParser8211(ParseAll)
{
  setCell(cell);
}

void CellParser8211GDAL::parseS57CellIntern(QString cellName)
{
    //**** Open ENC, read the DDR ****
    if( !oModule.Open(cellName.toLatin1()))
    {
        throw QString("ERROR: Cannot open S-57 file: %1").arg(cellName);
    }

    //**** Read all DataRecords ****
    DDFRecord       *poRecord;
    long nStartLoc;
    int recNo = 0;
    nStartLoc = VSIFTell( oModule.GetFP() );
    for( poRecord = oModule.ReadRecord();
         poRecord != NULL; poRecord = oModule.ReadRecord(), ++recNo)
    {
        if (recNo <= 1) ParseHeaderRecord(*poRecord);
        else 
        nStartLoc = VSIFTell(oModule.GetFP() );
    }
    oModule.Close();
  
#ifdef DBMALLOC
    malloc_dump(1);
#endif
}

void CellParser8211GDAL::ParseHeaderRecord(DDFRecord & rec)
{

    for (int subI = 0; subI < rec.GetFieldCount(); ++subI)
    {
        DDFField * recFld =  rec.GetField(subI);
        const char * tag = recFld->GetFieldDefn()->GetName();
        if (strncmp(tag, "DSID", sizeof(tag)) == 0)
        {

        }
        else if (strncmp(tag, "DSSI", sizeof(tag)) == 0)
        {


        }
        else if (strncmp(tag, "DSPM", sizeof(tag)) == 0)
        {


        }
    }
}

void CellParser8211GDAL::ParseFeatureRecord(DDFRecord & rec)
{


}