//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "catalog031reader.h"
#include "iso8211_simple.h"

#include <QtCore/QFile>

#include <math.h>

using namespace Enc;

//const int CatalogEntry::MemberCnt;

//******************************************************************************
/// Constructor initializes all data to Null-like-data
//******************************************************************************
CatalogEntry::CatalogEntry() : recId8211(0), RCID(0), CRCS(0)
{
    memset(RCNM,0,sizeof(RCNM));
    memset(IMPL,0,sizeof(IMPL));
}

//******************************************************************************
/// Treat the Calalog-struct like a vector
/*!
***************************************************************************** */
QVariant CatalogEntry::operator[](int index)const
{
    if (index == COL_RECID8211) return recId8211; //QString::number(recId8211);
    else if (index == COL_RCNM) return QString(RCNM);
    else if (index == COL_RCID) return RCID; //QString::number(RCID);
    else if (index == COL_FILE) return QString(FILE);
    else if (index == COL_LFIL) return QString(LFIL);
    else if (index == COL_VOLM) return QString(VOLM);
    else if (index == COL_IMPL) return QString(IMPL);
    else if (index == COL_bBox) return bBox.toString();
    else if (index == COL_CRCS) return (CRCS?QString::number(CRCS) : QString(""));
    else if (index == COL_COMT) return QString(COMT);
  
    return QVariant();
}

//******************************************************************************
/// Parse one ISO8211 S-57 Catalog record
/*!
***************************************************************************** */
void CatalogEntry::parseCatalogRecord(const char * catRecord)
{
    //**** 1) parse the leader ****
    unsigned int reclen, recFieldStart, sizeofFieldLen, sizeofFieldPos;
    if (!ISO8211::parseLeader(catRecord, &reclen, &recFieldStart, &sizeofFieldLen, &sizeofFieldPos))
    {
        throw QString("Cannot parse leader of catalog record");
    }
    //**** 2) parse the directory ****
    const char * dirPtr = catRecord + 24;  //leader is always 24 byte
    bool numOk;
    int fLen0001 = QString::fromAscii(dirPtr +4, sizeofFieldLen).toUInt(&numOk); 
    int fLenCADT = QString::fromAscii(dirPtr +4 +sizeofFieldLen +sizeofFieldPos +4, sizeofFieldLen).toUInt(&numOk);
    int fPosCADT = QString::fromAscii(dirPtr +4 +sizeofFieldLen +sizeofFieldPos +4 +sizeofFieldLen, sizeofFieldPos).toUInt(&numOk);

    //**** 3) parse the "0001" field (ISO8211 rec-id) - not really needed ****
    int FTind = ISO8211::findCharOffset(catRecord + recFieldStart, ISO8211::FT, catRecord + recFieldStart + fLen0001);
    if (FTind <= 0 || FTind > fLen0001) FTind = fLen0001; //will work even if FT was "forgotten"
    recId8211 = QString::fromAscii(catRecord + recFieldStart, FTind).toUInt(&numOk);
    if (!numOk)
    {
        qWarning("WARNING: iso8211 record id unreadable! ");//this is just a minor error!
    }

    //**** 4) parse the "CATD" field ****
    parseCATD(catRecord + recFieldStart + fPosCADT, fLenCADT);
}

//******************************************************************************
/// Parse a CATD Field in an ISO8211 S-57 Catalog record
/*!
***************************************************************************** */
void CatalogEntry::parseCATD(const char * fieldPtr, unsigned int fieldLen)
{
    const char * endPtr = fieldPtr + fieldLen;
    bool numOk;

    // 1) RCNM (RecordName) 2-char
    strncpy(RCNM, fieldPtr, 2);
    RCNM[2] = 0;
    fieldPtr += 2;

    // 2) RCID
    RCID = QString::fromAscii(fieldPtr, 10).toUInt(&numOk);
    fieldPtr += 10;

    // 3) FileName (with Path)
    int strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    FILE = QString::fromAscii(fieldPtr, strLength); 
    fieldPtr += strLength +1;

    // 4)File Long Name (may be empty)
    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    LFIL = QString::fromAscii(fieldPtr, strLength); 
    fieldPtr += strLength +1;

    // 5) Volume
    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    VOLM = QString::fromAscii(fieldPtr, strLength);
    fieldPtr += strLength +1;

    // 6) IMPLementation
    strncpy(IMPL, fieldPtr, 3);
    IMPL[3] = 0;
    fieldPtr += 3;

    // 7-10) Bounding Box: sLat, wLon, nLat, eLon
    bool ok1, ok2, ok3, ok4;
    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    bBox.SLAT = QString::fromAscii(fieldPtr, strLength).toDouble(&ok1);
    fieldPtr += strLength +1;

    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    bBox.WLON = QString::fromAscii(fieldPtr, strLength).toDouble(&ok2);
    fieldPtr += strLength +1;

    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    bBox.NLAT = QString::fromAscii(fieldPtr, strLength).toDouble(&ok3);
    fieldPtr += strLength +1;

    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    bBox.ELON = QString::fromAscii(fieldPtr, strLength).toDouble(&ok4);
    fieldPtr += strLength +1;

    if (!ok1 || !ok2 || !ok3 || !ok4) bBox.clear();

    // 11) CRC - rem: crc is stored as string of 8 hex digits 
    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    if (strLength > 0) CRCS = QString::fromAscii(fieldPtr, strLength).toUInt(&numOk, 16);
    fieldPtr += strLength +1;

    // 12) comment
    strLength = ISO8211::findCharOffset(fieldPtr, ISO8211::UT, endPtr);
    COMT = QString::fromAscii(fieldPtr, strLength);
}

//******************************************************************************
/// Calculate Size of CADT field of ISO8211 S-57 Catalog record in Bytes
/*!
* usefull to allocate space for a catalog.031 file to write
* rem: format is "A(2),I(10),3A,A(3),4R,2A" -> but some fields are fixed,
*      but others need a UT or FT, which needs 1 Byte 
***************************************************************************** */
int CatalogEntry::calcCADTsize(int posPrecision) const
{
    //** add the values 
    int size = 2+10  //recordId
      +FILE.length() +1    //including UT
      +LFIL.length() +1    //including UT
      +VOLM.length() +1    //including UT
      + 3
      +COMT.length() +1    //including UT
      +1; //FT

    //crc might be empty!
    if (CRCS == 0) size += 1;    //only UT
    else           size += 8 +1; //assuming CRC is allways 8 hex digits, + UT

    //** bounding box: length depends on precision, sign and value and if pos is valid at all: **
    if (bBox.isValid())
    {
        size += 4 * (posPrecision +3); //consists of: one leading digit +punkt +mantissa +UT
        //** position might take more space: for leanding sign and leading digits
        if (bBox.SLAT < 0) size += 1;
        if (bBox.WLON < 0) size += 1;
        if (bBox.NLAT < 0) size += 1;
        if (bBox.ELON < 0) size += 1;
        if (fabs(bBox.SLAT) > 10.0) size += 1; //assuming that abs lat is never greater 90 :-)
        if (fabs(bBox.NLAT) > 10.0) size += 1;

        if (fabs(bBox.WLON) > 100.0) size += 2; //assuming that abs lon is never greater 180 :-)
        else if (fabs(bBox.WLON) > 10.0) size += 1;
        if (fabs(bBox.ELON) > 100.0) size += 2;
        else if (fabs(bBox.ELON) > 10.0) size += 1;
    }
    else size += 4; //only 4 UTs

    return size;
}

//******************************************************************************
/// Get the lenght of the longest subfield in the CATD Field
//******************************************************************************
int CatalogEntry::getMaxSubFieldSize() const
{
    int maxSFSize = 13;  //assume that pos is never longer than 13 
    if (FILE.length() > maxSFSize) maxSFSize = FILE.length();
    if (LFIL.length() > maxSFSize) maxSFSize = LFIL.length();     
    if (VOLM.length() > maxSFSize) maxSFSize = VOLM.length();
    if (COMT.length() > maxSFSize) maxSFSize = COMT.length(); 
    return maxSFSize;
}

//******************************************************************************
/// Write a complete ISO8211 S-57 Catalog record 
/*!
* Caller must take care that startPtr points to a field big enought for complete record!!
* if "sizeofFLF" or "sizeofFPF" they are set automatically 
***************************************************************************** */
void CatalogEntry::writeCatalogRecord(QTextStream & iso8211strm, unsigned int iso8211rcid, unsigned int field001size, int sizeofFLF, int sizeofFPF, int posPrecision) const
{
    //**** calc record-length ,fieldStart, field-sizes ....****
    int recLenCATD = calcCADTsize(posPrecision);
    if (sizeofFLF <= 0)
    {
      sizeofFLF = 2;
      if      (getMaxSubFieldSize() >= 10000) sizeofFLF = 6;
      else if (getMaxSubFieldSize() >= 100)   sizeofFLF = 4;
    }
    if (sizeofFPF <= 0)
    {
        sizeofFPF = 1;   //one is always enought, because we have only 2 fields, and the first is less than 9 chars
        if (field001size > 8) sizeofFPF = 2;
    }
    unsigned int dirSize = 2*4 + 2*sizeofFLF + 2*sizeofFPF + 1; //including FT
    unsigned int recLen = 24 + dirSize + field001size + recLenCATD; //including all FTs
    unsigned int recFieldStart = 24 + dirSize;

    //** write record leader 
    ISO8211::writeRecLeaderS57(iso8211strm, recLen, recFieldStart, sizeofFLF, sizeofFPF);
    //** write directory 
    ISO8211::writeRecDir4CatS57(iso8211strm, field001size, recLenCATD, sizeofFLF, sizeofFPF);
    //** write "0001" field 
    ISO8211::write0001fieldASC(iso8211strm, iso8211rcid, field001size);
    //** write "CATD" field 
    writeCATDfield(iso8211strm, posPrecision);
}

//******************************************************************************
/// Write the CATD Field (the one and only importand field of a Catalog-Record)  
/*!
* 
***************************************************************************** */
void CatalogEntry::writeCATDfield(QTextStream & iso8211strm, int posPrecision) const
{
    //RCNM
    iso8211strm.setFieldWidth(2);
    iso8211strm << RCNM;
    //Rec-ID
    iso8211strm.setFieldWidth(10);
    iso8211strm.setPadChar(QChar('0'));
    iso8211strm << RCID;
    iso8211strm.setFieldWidth(0);
    //Filename:
    iso8211strm << FILE << ISO8211::UT;
    //Long Filename:
    iso8211strm << LFIL << ISO8211::UT;
    //Volume Name
    iso8211strm << VOLM << ISO8211::UT;
    //Implementation (always "BIN" for S-57)
    iso8211strm.setFieldWidth(3);
    iso8211strm << IMPL;
    iso8211strm.setFieldWidth(0);
    //Bounding-Box: holds SLAT, WLON, NLAT, ELON - but: may be empty
    if (bBox.isValid())
    {
        iso8211strm.setRealNumberPrecision(posPrecision);
        iso8211strm.setRealNumberNotation(QTextStream::FixedNotation);
        iso8211strm << bBox.SLAT << ISO8211::UT;
        iso8211strm << bBox.WLON << ISO8211::UT;
        iso8211strm << bBox.NLAT << ISO8211::UT;
        iso8211strm << bBox.ELON << ISO8211::UT;
    }
    else
    {
        iso8211strm << ISO8211::UT << ISO8211::UT << ISO8211::UT << ISO8211::UT; 
    }
    //checksum
    if (CRCS != 0)
    {
        iso8211strm.setFieldWidth(8);
        iso8211strm.setPadChar(QChar('0'));
        iso8211strm << CRCS;
        iso8211strm.setFieldWidth(0);
    }
    iso8211strm << ISO8211::UT;
    //comment - Holds S-63 entries !!
    iso8211strm << COMT << ISO8211::UT << ISO8211::FT;

    //** reset field modifiers **
    iso8211strm.setFieldWidth(0);
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
/// Constructor - does nothing unti now 
/*!
* 
***************************************************************************** */
Catalog031reader::Catalog031reader()
{}

Catalog031reader::~Catalog031reader()
{}

//******************************************************************************
/// Read all Catalog entries of an ISO8211 S-57 Catalog  
/*!
* Throws exception on Error
* "allEntries" is cleared if new Catalog is opened sucessfully
***************************************************************************** */
void Catalog031reader::readCatalog031(QString fileName, std::vector<CatalogEntry> & allEntries)
{
    //**** load iso8211 file into memory ****
    QFile file8211Qf(fileName);
    if (!file8211Qf.open(QIODevice::ReadOnly))
    {
        throw  QString("Error: cannot open file: %1").arg(fileName);
    }

    //** whole file into memory **
    int fileSize = file8211Qf.size();
    char * fileBuffer = new char[fileSize];
    qint64 bypesRead = file8211Qf.read(fileBuffer, fileSize);
    file8211Qf.close();

    //**** Parse Catalog and fill vector ****
    allEntries.clear();
    Iso8211recordIterator rcIt;
    rcIt.init(fileBuffer, fileSize);
    
    for (; rcIt.current(); rcIt.next())
    {
        int currRecLen = 0;
        const char * currRec = rcIt.current(&currRecLen);
        CatalogEntry catEntry;
        catEntry.parseCatalogRecord(currRec);
        allEntries.push_back(catEntry);
        //qWarning(currRec);;
    }
}


//******************************************************************************
//******************************************************************************
//******************************************************************************
/// Constructor - does nothing unti now 
/*!
* 
***************************************************************************** */
Catalog031writer::Catalog031writer() 
{}

Catalog031writer::~Catalog031writer()
{}

void Catalog031writer::init()
{}

//******************************************************************************
/// Write a Catalog.031 
/*!
* Field size is calculated automatically
***************************************************************************** */
void Catalog031writer::writeCatalog031(QString fileName, std::vector<CatalogEntry> & allEntries)
{
    QFile iso8211file(fileName);
    if (!iso8211file.open(QIODevice::WriteOnly)) throw QString("ERROR: cannot open Catalog.031 for writing").arg(fileName);
    QTextStream outStrm(&iso8211file);

    //** write DDR **
    ISO8211::writeDDR4catS57(outStrm);

    //** caluclate appropriate field width **
    unsigned int iso8211rcid = 0;
    unsigned int field001size = 3;    //rem: 1Byte is needed for the FT!!!
    if      (allEntries.size() >= 100000) field001size = 11;
    else if (allEntries.size() >= 10000) field001size = 6;
    else if (allEntries.size() >= 1000) field001size = 5;
    else if (allEntries.size() >= 100) field001size = 4;

    int sizeofFLF =0, sizeofFPF =0, posPrecision = 8;  //used default values, for now

    //** write all catalog entries
    std::vector<CatalogEntry>::const_iterator recIt;
    for (recIt = allEntries.begin(); recIt != allEntries.end(); ++recIt)
    {
        (*recIt).writeCatalogRecord(outStrm, iso8211rcid, field001size,  sizeofFLF, sizeofFPF, posPrecision);
    }

    iso8211file.close();
}
