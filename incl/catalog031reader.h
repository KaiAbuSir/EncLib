#ifndef CATALOG031_READER_H
#define CATALOG031_READER_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>

#include <vector>

#include "boundingbox_degrees.h"

namespace Enc
{

//******************************************************************************
/// Holds all information off a record of a CATALOG.031 File (0001,CATD fields)
/*!
*
***************************************************************************** */
struct CatalogEntry
{
    const static unsigned int MemberCnt = 10;  //number of members in CatalogEntry-struct: 10
    enum ColumnNames{COL_RECID8211 = 0, COL_RCNM, COL_RCID, COL_FILE, COL_LFIL, COL_VOLM , COL_IMPL , COL_bBox , COL_CRCS , COL_COMT};
    QVariant operator[](int index)const;
    //QString  getToolTip(int index)const;

    CatalogEntry();
    //** 0001-field: ISO8211 record id - unimportant, but needed for writing **
    unsigned int recId8211;

    //** catalog record: data of CATD field **

    char          RCNM[4];  //Recored Name always "DS" - unimportant
    unsigned int  RCID;     //Record ID - unimportant
    
    QString       FILE;     //Filename
    QString       LFIL;     //Long Filename 
    QString       VOLM;     //Volume Name
    char          IMPL[4];  //Implementation "ASC" or "BIN" (always "BIN" for S-57)
    DegBBox       bBox;     //holds SLAT, WLON, NLAT, ELON
    unsigned int  CRCS ;     //checksum - 0 means: no checksum, althought 0 is a possibble checksum
    QString       COMT;     //comment - Holds S-63 entries !!

    //** methods needed for parsing iso8211 record **
    void parseCatalogRecord(const char * catRecord);
    void parseCATD(const char * fieldPtr, unsigned int fieldLen);

    //** methods needed for writing iso8211 record **
    int calcCADTsize(int posPrecision) const;  
    int getMaxSubFieldSize() const;
    void writeCatalogRecord(QTextStream & iso8211strm, unsigned int iso8211rcid, unsigned int field001size = 6, int sizeofFLF =0, int sizeofFPF =0, int posPrecision = 8) const;
    void writeCATDfield(QTextStream & iso8211strm, int posPrecision) const;
};

//******************************************************************************
/// Holds  information off a record of a CATALOG.031 File, plus check-Result
/*!
* 
***************************************************************************** */
struct CatalogCheckEntry : public CatalogEntry
{
    enum CheckStatus{UnChecked =0, Ok, Warn, Err};

    CatalogCheckEntry() : CatalogEntry(), chkStat(UnChecked) {}
    CheckStatus chkStat;
    QStringList messages;
};

//******************************************************************************
/// Reads a S-57 CATALOG.031 File
/*!
***************************************************************************** */
class Catalog031reader
{
public:
    Catalog031reader();
    ~Catalog031reader();
    void readCatalog031(QString fileName, std::vector<CatalogEntry> & allEntries);

private:

};

//******************************************************************************
/// Write a S-57 CATALOG.031 File
/*!
***************************************************************************** */
class Catalog031writer
{
public:
    Catalog031writer();
    ~Catalog031writer();
    void init();
    void writeCatalog031(QString fileName, std::vector<CatalogEntry> & allEntries);
 
private:

};
}

#endif

