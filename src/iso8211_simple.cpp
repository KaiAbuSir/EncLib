#include <QString>

#include <math.h>

#include "iso8211_simple.h"

using namespace Enc;

//ISO8211::UT = 0x1F;
//ISO8211::FT = 0x1E;



//******************************************************************************
/// Calculate minimal number of decimal digits needed to write a number
/*! 
 *  int(log(num)) +1
 ****************************************************************************** */
int  ISO8211::decDigits4Number(long long num)
{
    int digits = int(log10(double(num))) +1;
    if (num < 0) ++digits;
    return digits;
}

//******************************************************************************
/// Convert a int number to a fixed lenth string, no matter how big "num" is
//****************************************************************************** */
QString ISO8211::number2String(long long num, int digits)
{
    QString str = QString::number(num);
    if (str.length() > digits) throw QString("Cannot convert Number %1 to fixed length string: Number to big!").arg(num);
    else if (str.length() < digits)
    {
        str.prepend(QString(digits - str.length(), QChar('0')));
    }
    return str;
}

//******************************************************************************
/// little helper function converting string to unsigned long
/*! needed because stdlib functions to not take "len" parameter,
 *  but convert until next notAnumber-char is met 
 ****************************************************************************** */
unsigned long ISO8211::str2uInt(const char * startPtr, int len)
{
    if (len < 1 || len > 10) throw QString("ERROR: Impossible conversion string to long: %1").arg(QString::fromLatin1(startPtr, len));
    long val = 0;
    for (int i = 0; i < len; ++i)
    {
        val *= 10;
        if ( *(startPtr +i) < 0x30 || *(startPtr +i) > 0x39)
        {
            throw QString("ERROR: Impossible conversion string to long: Not a number: %1").arg(QString::fromLatin1(startPtr, len));
        }
        val +=  *(startPtr +i) - 0x30;
    }
    return val;
}

//******************************************************************************
/// little helper function to find Terminator-characters other than NULL 
//******************************************************************************
int ISO8211::findCharOffset(const char * startPos, char findMe, const char * end)
{
    const char * myPos = startPos;
    for(;myPos <= end; ++myPos){
        if(*myPos == findMe) return myPos - startPos;
    }
    throw QString("Error: cannot find field Terminator in %1").arg(QString::fromLatin1(startPos, end - startPos));
}

//******************************************************************************
/// little helper function to find UTF-16 Terminator-characters other than NULL 
//******************************************************************************
int ISO8211::findCharOffset(const char * startPtr, wchar_t findMe, const char * end)
{
    const wchar_t * startPos = reinterpret_cast<const wchar_t *>(startPtr);
    const wchar_t * endPos = reinterpret_cast<const wchar_t *>(end);
    const wchar_t * myPos = startPos;
    for(;myPos <= endPos; ++myPos){
        if(*myPos == findMe) return myPos - startPos;
    }
    throw QString("Error: cannot find utf16 field Terminator!");
}

//******************************************************************************
/// little helper function to find Terminator-characters other than NULL 
//******************************************************************************
const char * ISO8211::findCharPosition(const char * startPos, char findMe, const char * end)
{
    const char * myPos = startPos;
    for(;myPos <= end; ++myPos){
        if(*myPos == findMe) return myPos;
    }
    throw QString("Error: cannot find field Terminator!");
}

//******************************************************************************
/// check iso8211 record of an S-57 file, and get rec-length, etc... if desired
/*!
* Since some leader values are fixed in S-57, they are only checked, but not parsed!
* Usefull to find out if a File is a iso8211 file at all
***************************************************************************** */
bool ISO8211::checkDDRleader(const char * iso8211content, unsigned int fileSize, 
                                           unsigned int * DDRlen, unsigned int * DDRfieldStart, unsigned int * sizeofFieldLen, unsigned int * sizeofFieldPos)
{
    if (iso8211content == 0 || fileSize < 24) return false;
    if(!parseLeader(iso8211content, DDRlen, DDRfieldStart, sizeofFieldLen, sizeofFieldPos)) return false;

    //** check the fixed part of the leader **
    if (strncmp(iso8211content +5, "3LE1 09", 7) != 0) return false;
    if (strncmp(iso8211content +17, " ! ", 3) != 0) return false;
    if (strncmp(iso8211content +22, "04", 2) != 0) return false; //rem: in S-57, tag size is ALWAYS 4 !

    return true;
}

//******************************************************************************
/// parse/check iso8211 record-leader of an S-57 file
/*!
* Only leader values, that are not fixed in S-57, are parsed / checked!
* User has to take care that recPtr is valid, means: points to a field of 24 Bytes!
***************************************************************************** */
bool ISO8211::parseLeader(const char * recPtr, unsigned int * reclen, unsigned int * recFieldStart, unsigned int * sizeofFieldLen, unsigned int * sizeofFieldPos)
{
    if (reclen) *reclen = 0;
    if (recFieldStart) *recFieldStart   = 0;
    if (sizeofFieldLen) *sizeofFieldLen =0;
    if (sizeofFieldPos) *sizeofFieldPos =0;

    if (recPtr == NULL) return false;

    //** check record length field **
    try
    {
        unsigned long myRecLen = ISO8211::str2uInt(recPtr, 5); //= QString::fromAscii(recPtr, 5).toUInt(&numOk); 
        if (reclen) *reclen = myRecLen;
    }
    catch(...)
    {
        return false;
    }

    //** check data start offset field **
    try
    {
        unsigned long myFieldStart = ISO8211::str2uInt(recPtr +12, 5); //QString::fromAscii(recPtr +12, 5).toUInt(&numOk); 
        if (recFieldStart) *recFieldStart = myFieldStart;
    }
    catch(...)
    {
        return false;
    }

    //** check length of the length/position length-fields **
    int digit = QChar(*(recPtr +20)).digitValue();
    if (digit <= 0) return false;
    else if (sizeofFieldLen) *sizeofFieldLen = (unsigned int)digit;
    
    digit = QChar(*(recPtr +21)).digitValue();
    if (digit <= 0) return false;
    else if (sizeofFieldPos) *sizeofFieldPos = (unsigned int)digit;

    return true;
}

//******************************************************************************
/// Parses an Iso8211-Record-Directory (suitable for S-57 files, only!)
/*!
* Rem: In S-57, the Tag is always 4 characters long
***************************************************************************** */
void ISO8211::parseDirectory(std::vector<ISO8211::DirEntry> & dirEntries, const char * ptr, int len, unsigned int sizeofFieldLen, unsigned int sizeofFieldPos)
{
    const char * endPtr = ptr + len;
    const int incr = 4 + sizeofFieldLen + sizeofFieldPos;
    dirEntries.reserve(len / incr);  //avoid resizing 
    for (;ptr < endPtr; ptr += incr)
    {
        if (*ptr == ISO8211::FT) break;  //maybe FieldTerminator is included in "len"
        ISO8211::DirEntry entry;
        strncpy(entry.tag, ptr, 4); //Tag: 4 Bytes, allways
        entry.fieldLen = ISO8211::str2uInt(ptr +4, sizeofFieldLen);
        entry.fieldPos = ISO8211::str2uInt(ptr +4 +sizeofFieldLen, sizeofFieldPos);
        dirEntries.push_back(entry);
    }
}

//******************************************************************************
/// Write the Leader of the DDR (first iso8211 record) for an S-57 file
/*!
* User has to take care that recPtr points to a field big enought for leader
* Returns pointer to first Byte after leader
***************************************************************************** */
void ISO8211::writeDDRleaderS57(QTextStream & iso8211strm, unsigned int recLen, unsigned int recFieldStart, unsigned int sizeofFieldLen, unsigned int sizeofFieldPos)
{
    char buf[28];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%05d3LE1 09%05d ! %1d%1d04", recLen, recFieldStart, sizeofFieldLen, sizeofFieldPos);
    iso8211strm << buf;
}
 
//******************************************************************************
/// Write the the complete DDR of an iso8211 S-57-Catalog file
/*!
* size fields could be variable, but to make is simple: 
* sizeofFieldLen =3 sizeofFieldPos =3 are sufficiant, because DDR of catalog-file is short
*
* Returns pointer to first Byte after leader.
***************************************************************************** */
void ISO8211::writeDDR4catS57(QTextStream & iso8211strm)
{
    unsigned int sizeofFieldLen =3, sizeofFieldPos = 3;
    unsigned int recFieldStart = 55; //=24 + 3 * 10 +1;  //ddr-length + dir-length
    unsigned int recLen = 238; //55 + 183 Bytes
    
    writeDDRleaderS57(iso8211strm, recLen, recFieldStart, sizeofFieldLen, sizeofFieldPos);

    //** write the DDR directory: length of 1 Entry = 4 + sizeofFieldLen + sizeofFieldPos **
    iso8211strm << "0000019000";       //Field Control Field -dirEntry: 19 Bytes, pos. 0
    iso8211strm << "0001044019";       //iso8211-recid Descriptive Field -dirEntry: 44 Bytes, pos. 19
    iso8211strm << "CATD120063" << FT;  //CADT  Descriptive Field -dirEntry: 120 Bytes, pos. 63

    //** the reocrds of the DDR: 19 + 44 + 120 Byte = 183 Bytes (incl Termiators)
    iso8211strm << "0000;&   " << UT << "0001CATD" << FT;                             //** Field Control Field - ALWAYS 19 Byte
    iso8211strm << "0100;&   ISO 8211 Record Identifier" << UT << UT << "(I(5))" << FT; //** Data Descriptive Field for Tag "0001" - ALWAYS 44 Byte
    iso8211strm << "1600;&   Catalog Directory field" << UT << "RCNM!RCID!FILE!LFIL!VOLM!IMPL!SLAT!WLON!NLAT!ELON!CRCS!COMT" << UT << "(A(2),I(10),3A,A(3),4R,2A)" << FT; //Data Descriptive Field for Tag "CATD" - ALWAYS 120 Byte 
}

//******************************************************************************
/// Write the Directory of a Calalog Record for an S-57 file
/*!
* Write the direcory of a S-57 Catalog-Entry to TextStream
* A Catalog-Record-Dir has only 2 entries: "0001" and "CADT"
***************************************************************************** */
void ISO8211::writeRecDir4CatS57(QTextStream & iso8211strm, unsigned int recLen0001, unsigned int recLenCATD, unsigned int sizeofFLenF, unsigned int sizeofFPosF)
{
    //** "0001" entry **
    iso8211strm.setPadChar(QChar('0'));
    iso8211strm << "0001";
    iso8211strm.setFieldWidth(sizeofFLenF);
    iso8211strm << recLen0001;
    iso8211strm.setFieldWidth(sizeofFPosF);
    iso8211strm << 0; 

    //** CATD entry **
    iso8211strm << "CATD";
    iso8211strm.setFieldWidth(sizeofFLenF);
    iso8211strm << recLenCATD;
    iso8211strm.setFieldWidth(sizeofFPosF);
    iso8211strm << recLen0001;  //the record length of "0001" field is at the same time the start-pos. of CATD field
    
    iso8211strm.setFieldWidth(0);
    iso8211strm.setPadChar(QChar(' '));

    iso8211strm <<  FT;  
}

//******************************************************************************
/// Write the leader of a normal Record of an iso8211 S-57 file
/*!
* User has to take care that recPtr points to a field big enought for leader
* Returns pointer to first Byte after leader
***************************************************************************** */
void ISO8211::writeRecLeaderS57(QTextStream & iso8211strm, unsigned int recLen, unsigned int recFieldStart, unsigned int sizeofFieldLen, unsigned int sizeofFieldPos)
{
    char buf[28];
    memset(buf,0,sizeof(buf));
    sprintf(buf, "%05d D     %05d   %1d%1d04", recLen, recFieldStart, sizeofFieldLen, sizeofFieldPos);
    iso8211strm << buf;
}

//******************************************************************************
/// Write the "0001" field (holds the Iso8211 record-id) to a ASCII Iso8211 file
/*!
* Rem: "field001size" is field size including Terminator (FT)
* Writes the recId and the FT (Field Terminator)
* Returns pointer to first Byte after leader
***************************************************************************** */
void ISO8211::write0001fieldASC(QTextStream & iso8211strm, unsigned int iso8211rcid, unsigned int field001size) 
{
    iso8211strm.setFieldWidth(field001size -1);
    iso8211strm.setPadChar(QChar('0'));
    iso8211strm << iso8211rcid;
    iso8211strm.setFieldWidth(0);
    iso8211strm.setPadChar(QChar(' '));
    iso8211strm <<  ISO8211::FT;
}

//******************************************************************************
/// Write the 0001 Field to a BINARY Iso8211 file
/*!
* bin RCNM is a 1 byte integer, ascii a 2 char token
***************************************************************************** */
unsigned long ISO8211::write0001fieldBIN(QIODevice * device, unsigned long currentIso8211rcid, unsigned long field001size)
{
    unsigned long bytesWritten = 0;
    if (field001size -1 == 1)
    {
        unsigned char num = currentIso8211rcid;
        bytesWritten += writeUChar(device, num);
    }
    else if (field001size -1 == 2)
    {
        //**** for S-57 ENCs, iso8211 rcid should ALWAYS be 2byte-int ****
        unsigned short num = currentIso8211rcid;
        bytesWritten +=  writeUShort(device, num);
    }
    else if (field001size -1 == 4)
    {
        unsigned long num = currentIso8211rcid;
        bytesWritten +=  writeULong(device, num);
    }
    else if (field001size -1 == 8)
    {
        unsigned long long num = currentIso8211rcid;
        bytesWritten +=  writeULongLong(device, num);
    }
    else
    {
        throw QString("ERROR: Invalid length of 0001 Field");
    
    }
    device->putChar(ISO8211::FT);
    ++bytesWritten;
    return bytesWritten; //make compiler happy
}

//******************************************************************************
/// Convert the binary Record-Name-ID to Ascii
/*!
* bin RCNM is a 1 byte integer, ascii a 2 char token
* see: S-57 Part 3, Chapter 2.2.1
***************************************************************************** */
const char * ISO8211::rcnm2ascii(const char * binPtr)
{
    const unsigned char * rcnmId = reinterpret_cast<const unsigned char *>(binPtr);
    return ISO8211::rcnm2ascii(*rcnmId);

}
const char * ISO8211::rcnm2ascii(unsigned char rcnmId)
{
    if (rcnmId == 10) return "DS";
    if (rcnmId == 20) return "DP";
    if (rcnmId == 30) return "DH";
    if (rcnmId == 40) return "DA";
    if (rcnmId == 60) return "CR";  //Catalog Cross Reference
    if (rcnmId == 70) return "ID";  //dict def field
    if (rcnmId == 80) return "IO";  //dict domain identifier
    if (rcnmId == 90) return "IS";  //dict schema identifier
    if (rcnmId == 100) return "FE";  //Feature Record identifier
    if (rcnmId == 110) return "VI";  //Spatial Record Isolated Node
    if (rcnmId == 120) return "VC";  //Spatial Record Connected Node
    if (rcnmId == 130) return "VE";  //Spatial Record Edge
    if (rcnmId == 140) return "VF";  //Spatial Record Face - forbidden!

    //** if nothing matches **
    return "  ";
}

unsigned char ISO8211::rcnmFromAscii(const char * rcnmAscii)
{
    if (rcnmAscii[0] == 'D' &&  rcnmAscii[1] == 'S') return 10;
    if (rcnmAscii[0] == 'D' &&  rcnmAscii[1] == 'P') return 20;
    if (rcnmAscii[0] == 'D' &&  rcnmAscii[1] == 'H') return 30;
    if (rcnmAscii[0] == 'D' &&  rcnmAscii[1] == 'A') return 40; // "DA";
    if (rcnmAscii[0] == 'C' &&  rcnmAscii[1] == 'R') return 60; // "CR";  //Catalog Cross Reference
    if (rcnmAscii[0] == 'I' &&  rcnmAscii[1] == 'D') return 70; // "ID";  //dict def field
    if (rcnmAscii[0] == 'I' &&  rcnmAscii[1] == 'O') return 80; // "IO";  //dict domain identifier
    if (rcnmAscii[0] == 'I' &&  rcnmAscii[1] == 'S') return 90; // "IS";  //dict schema identifier
    if (rcnmAscii[0] == 'F' &&  rcnmAscii[1] == 'E') return 100; // "FE";  //Feature Record identifier
    if (rcnmAscii[0] == 'V' &&  rcnmAscii[1] == 'I') return 110; // "VI";  //Spatial Record Isolated Node
    if (rcnmAscii[0] == 'V' &&  rcnmAscii[1] == 'C') return 120; // "VC";  //Spatial Record Connected Node
    if (rcnmAscii[0] == 'V' &&  rcnmAscii[1] == 'E') return 130; // "VE";  //Spatial Record Edge
    if (rcnmAscii[0] == 'V' &&  rcnmAscii[1] == 'F') return 140; // "VF";  //Spatial Record Face - forbidden!

    return 0;
}

QString ISO8211::makeRecNameASCII(unsigned char rcnm, unsigned long rcid)
{
    return QString("%1/%2").arg(QString(rcnm2ascii(rcnm)),2, QLatin1Char('_')).arg(rcid, 10, 10, QLatin1Char('0'));
}
QString ISO8211::makeRecNameASCII(RecNAME recName)
{
    unsigned long rcid = static_cast<unsigned long>(recName); //just cut parts over 4 Bytes by casting to 4Byte int
    unsigned char rcnm = recName / 0x100000000;
    return makeRecNameASCII(rcnm, recName);
}

///Make readable string from LongName, likeL AG=12FIDN=1234,FIDS=12
QString ISO8211::makeLongNAMeASCII(LongNAMe lName)
{
    unsigned short AG =0, FIDS =0; 
    unsigned long FIDN =0; 
    unmakeLongNAMe(lName, AG, FIDN, FIDS);
    return QString("AG=%1 FIDN=%2 FIDS=%3").arg(AG,5,10,QChar('0')).arg(FIDN,5,10,QChar('0')).arg(FIDS,5,10,QChar('0'));
}

//******************************************************************************
/// Convert the binary Product-Specification-ID to Ascii
/*!
* bin RCNM is a 1 byte integer, ascii a 3 char token
***************************************************************************** */
const char * ISO8211::prsp2ascii(const char * binPtr)
{
    const unsigned char * idPtr = reinterpret_cast<const unsigned char *>(binPtr);
    return ISO8211::prsp2ascii(*idPtr);

}
const char * ISO8211::prsp2ascii(unsigned char id)
{
    if (id == 1) return "ENC";
    if (id == 2) return "ODD";
    return "   ";
}

//******************************************************************************
/// Convert the binary Application-Profile-Spec-ID to Ascii
/*!
* bin PROF is a 1 byte integer, ascii a 2 char token
***************************************************************************** */
const char * ISO8211::prof2ascii(const char * binPtr)
{
    const unsigned char * idPtr = reinterpret_cast<const unsigned char *>(binPtr);
    return prof2ascii(*idPtr);
}
const char * ISO8211::prof2ascii(unsigned char id)
{
    if (id == 1) return "EN";
    if (id == 2) return "ER";
    if (id == 3) return "DD";
    return "   ";
}

//******************************************************************************
/// Convert a binary 16bit unicode string
/*!
* If no Length is given, the ISO8211 UnitTerminator is assumed - a little bit dangerous
***************************************************************************** */
QString ISO8211::parseLexLevel2(const char * binPtr, int len)
{
    return QString::fromUtf16 ((reinterpret_cast<const ushort *>(binPtr)), len);
    //attrib.value = QString::fromWCharArray ((reinterpret_cast<const ushort *>(fieldPtr)), offset); //needs special compiled Qt
}

QString ISO8211::parseLexLevel2(const char * binPtr)
{
    int len = ISO8211::findCharOffset(binPtr, ISO8211::UTL2, NULL);
    return QString::fromUtf16 ((reinterpret_cast<const ushort *>(binPtr)), len);
    //attrib.value = QString::fromWCharArray ((reinterpret_cast<const ushort *>(fieldPtr)), offset); //needs special compiled Qt
}


//******************************************************************************
//******************************************************************************

//******************************************************************************
/// Human readable Form of the S-57 3-char IMPL field
//******************************************************************************

QString  S57::EXPP2string(int expp)
{
    if      (expp == 1) return QString("1 (New)");
    else if (expp == 2) return QString("2 (Revision)");
    else
    {
        return QString("Unknown");
    }
}

QString  S57::PROF2string(int prof)
{
    if      (prof == 1) return QString("1 = EN (Base), binary");
    else if (prof == 2) return QString("2 = ER (Update), binary");
    else if (prof == 3) return QString("2 = DD (Data Dictionary)");
    
    return QString("%1 = Unknown").arg(prof);
}

QString S57::INTU2string(int intu)
{
    if (intu == 1) return QString("1 Overview");
    if (intu == 2) return QString("2 General");
    if (intu == 3) return QString("3 Coastal");
    if (intu == 4) return QString("4 Approach");
    if (intu == 5) return QString("5 Harbour");
    if (intu == 6) return QString("6 Berthing");
    return QString::number(intu);
}

QString S57::PRSP2string(int prsp)
{
    if (prsp == 1) return QString("1 ENC");
    return QString("%1 (Unknown code)").arg(prsp);
}


QString S57::IMPL2string(QString impl)
{
    impl = impl.toUpper();
    if (impl == "BIN") return QObject::tr("S-57 Base or Update File (Chart or Chart correction)");
    else if (impl == "ASC") return QObject::tr("S-57 Exchange Set Catalog file (CATALOG.031)\nor README file");
    else if (impl == "TXT") return QObject::tr("Text file");
    else if (impl == "TIF") return QObject::tr("TIFF Picture File");
    else if (impl == "PDF") return QObject::tr("PDF File");
    else if (impl == "HTM") return QObject::tr("HTML File");
    else if (impl == "JPG") return QObject::tr("JPEG Picture File");
    else if (impl == "AVI") return QObject::tr("AVI Audio/Video File");
    else if (impl == "MPG") return QObject::tr("MPEG Video File");
    return QString();
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
/// Constructor - only inits
//******************************************************************************
Iso8211recordIterator::Iso8211recordIterator() : startPtr(0), totalSize(0), recPtr(0), recSize(0)
{}

Iso8211recordIterator::~Iso8211recordIterator()
{}

//******************************************************************************
/// Skip DDR and set iterator to the first iso8211 record
/*!
* will only succeed it next recored can be read COMPLETELY!
***************************************************************************** */
void Iso8211recordIterator::init(const char * _startPtr, unsigned int _size)
{
    //** init temp data **
    recVec.clear();
    recSize = 0;

    //** save file data **
    startPtr = _startPtr;
    totalSize = _size;
  
    //** evaluate and check DDR **
    recPtr = startPtr;
    bool numOk;
    ddrRecLen = QString::fromAscii(_startPtr, 5).toUInt(&numOk); 
    if (!numOk)
    {
        throw QString("ERROR: Cannot read ISO8211: invalid DDR record length in %!").arg(QString::fromLatin1(_startPtr,24));
    }
    recPtr += ddrRecLen;
    //** if size is smaller than DDR -> finish with error **
    if (startPtr + totalSize < recPtr)
    {
      throw QString("ERROR: Cannot read ISO8211: data smaller than DDR record length in %1").arg(QString::fromLatin1(_startPtr,24));
    }
    recSize = QString::fromAscii(recPtr, 5).toUInt(&numOk); 
}

//******************************************************************************
/// Reset iterator to first record
//**************************************************************************** */
void Iso8211recordIterator::rewind()
{
  init(startPtr, totalSize);
}

//******************************************************************************
/// Count number of real records (=skip ddr) and init record
//**************************************************************************** */
unsigned int Iso8211recordIterator::recCount()
{
    if (startPtr == NULL) return 0;
    if (!recVec.empty()) return recVec.size();

    const char * myRecPtr = startPtr + ddrRecLen;  //skip DDR Recored
    const char * myEndPtr = startPtr + totalSize -1;
    bool numOk;
    while(myRecPtr +5 < myEndPtr )
    {
        recVec.push_back(myRecPtr);
        unsigned int myRecSize = QString::fromAscii(myRecPtr, 5).toUInt(&numOk);
        if (!numOk){
            throw QString("ERROR: Cannot read ISO8211: record length not readable");}
        myRecPtr += myRecSize;
    }
    return recVec.size();
}

//******************************************************************************
/// 
//**************************************************************************** */
const char * Iso8211recordIterator::current(int * _recSize) const
{
    if (_recSize) *_recSize = recSize;
    return recPtr;
}

//******************************************************************************
/// Get Pointer to the DDR (the first Record)
//**************************************************************************** */
const char * Iso8211recordIterator::getDDR(int * ddrSize) const
{
    if (ddrSize) *ddrSize = ddrRecLen;
    return startPtr;
}
 
//******************************************************************************
/// Direct access to a record - inits record-vector if not yet done
//**************************************************************************** */
const char * Iso8211recordIterator::at(unsigned int recInd, int * _recSize)
{
    if (recInd >= recCount()) //will init vector, if not yet done!
    {
        if (_recSize) *_recSize = 0;
        return NULL;
    }
    
    const char * recPtrAt = recVec[recInd];
    if (_recSize)
    {
        bool numOk;
        QString::fromAscii(recPtrAt, 5).toUInt(&numOk); 
    }
    return recPtrAt;
}

//******************************************************************************
/// set iterator to the next iso8211 record
/*!
* will only succeed it next recored can be read COMPLETELY!
***************************************************************************** */
void Iso8211recordIterator::next()
{
    recPtr += recSize;
    if (recPtr +5 <= startPtr+totalSize)
    {
       
        recSize = ISO8211::str2uInt(recPtr, 5); // QString::fromAscii(recPtr, 5).toUInt(&numOk);
        //** Special case: if record is very big (>99999bytes) it is set to ZERO and the directory contains the length **
        if (recSize == 0)
        {
            recSize = Iso8211fieldIterator(recPtr).getRecordLength();
        }
        //** Special case: Record is not complete -> dont use it! **
        //** this can happen if iso8211 file was read incompletely **
        if (recPtr + recSize > startPtr+totalSize)
        {
#ifdef _DEBUG
            printf("DEBUG: Record incomplete - Stopping!\n");
#endif
            recPtr = 0;  
            recSize = 0;
        }
    }
    else
    {
        recPtr = 0;  
        recSize = 0;
    }   
}

//******************************************************************************    
//******************************************************************************
//******************************************************************************
/// Constructor - Reads the Record Leader and the Record Directory
/*!
***************************************************************************** */
Iso8211fieldIterator::Iso8211fieldIterator(const char * recordStartPtr) : fieldStartPtr(0), fieldStartOffset(0), recSize(0), currentIndex(0)
{
    unsigned int sizeofFieldLen = 0, sizeofFieldPos =0;
    if (!ISO8211::parseLeader(recordStartPtr, &recSize, &fieldStartOffset, &sizeofFieldLen, &sizeofFieldPos))
    {
        throw QString("Error: Cannot parse ISO8211 record leader: %1").arg(QString::fromLatin1(recordStartPtr,25));
    }
    ISO8211::parseDirectory(dirEntries, recordStartPtr + 24, fieldStartOffset -24, sizeofFieldLen, sizeofFieldPos);
    fieldStartPtr = recordStartPtr + fieldStartOffset;
    if (recSize == 0)
    {
        recSize = checkLength();
    }
}

//******************************************************************************
/// Check if position/lenth entries in Dir make sense
/*!
* Returns the record Length
* Usefull for very long records, where Leader has 0-length
***************************************************************************** */
unsigned long Iso8211fieldIterator::checkLength() const
{
    unsigned int fieldLenSum = 0;
    for (uint i = 0; i < dirEntries.size(); ++i)
    {
        if (dirEntries[i].fieldPos != fieldLenSum) 
        {
            throw QString("ERROR: Record Directory corrupted: %1 not %2").arg(dirEntries[i].fieldPos).arg(fieldLenSum);
        }
        fieldLenSum +=   dirEntries[i].fieldLen; 
    }
 
    if (recSize > 0 && recSize != fieldStartOffset + fieldLenSum)
    {
        throw QString("ERROR: Record Directory corrupted, or wrong Record Length: %1 not %2").arg(recSize).arg(fieldStartOffset + fieldLenSum);
    }
    return fieldStartOffset + fieldLenSum;
}

const char * Iso8211fieldIterator::at(unsigned int fieldInd, int * recSize, const char *& tag)
{
    if (fieldInd < 0 || fieldInd >= dirEntries.size()) return 0;
    const ISO8211::DirEntry & entry = dirEntries[fieldInd];
    if (recSize) *recSize = entry.fieldLen;
    tag = entry.tag;
    currentIndex = fieldInd;
    return fieldStartPtr + entry.fieldPos;
}

const char * Iso8211fieldIterator::operator[](const char * fieldTag)
{
    std::vector< ISO8211::DirEntry >::iterator entryIt = dirEntries.begin();
    for (;entryIt != dirEntries.end(); ++entryIt)
    {
        if (strncmp(fieldTag, (*entryIt).tag, 4) == 0)
        {
            return fieldStartPtr + (*entryIt).fieldPos;
        }
    }
    return NULL;
}

const char * Iso8211fieldIterator::operator[](unsigned int fieldInd)
{
    if (fieldInd < 0 || fieldInd >= dirEntries.size()) return 0;
    currentIndex = fieldInd;
    return fieldStartPtr + dirEntries[fieldInd].fieldPos;
}

Iso8211fieldIterator::~Iso8211fieldIterator()
{}



    
    