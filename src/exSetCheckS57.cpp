//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "exSetCheckS57.h"

#include "catalog031reader.h"

using namespace Enc;

ExSetCheckS57::ExSetCheckS57() : checkEntries(NULL)
{

}

ExSetCheckS57::~ExSetCheckS57()
{}

void ExSetCheckS57::init(QString catFileAndPath, std::vector<CatalogCheckEntry> * catEntries)
{
    catFilePath = catFileAndPath;
    checkEntries = catEntries;
    checkErrors = CheckErrors();
}

void ExSetCheckS57::checkAll()
{
    //**** check parent-dir ****

    //**** check all entries ****
    std::vector<CatalogCheckEntry>::iterator entryIt;
    for (entryIt = checkEntries->begin(); entryIt != checkEntries->end(); ++entryIt)
    {
        checkEntry(*entryIt);

    }
}

void ExSetCheckS57::checkSome(std::set<int> indices2check)
{
    if (checkEntries== NULL || *indices2check.begin() < 0 || *indices2check.end() >= checkEntries->size())
    {
        throw QString("INTERNAL ERROR: invalid indices while checking catalog records");
    }
    //**#################################################################################################################################
}

void ExSetCheckS57::checkEntry(CatalogCheckEntry & entry2check)
{
        //**** check if entry exists *****
    //**#################################################################################################################################

        //**** check CRC ****
    //**#################################################################################################################################

        //**** check FileType ****
    //**#################################################################################################################################

}


