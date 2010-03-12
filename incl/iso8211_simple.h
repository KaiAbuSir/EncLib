#ifndef ISO_8211_SIMPLE_H
#define ISO_8211_SIMPLE_H

#include <QtCore/QString>
#include <QtCore/QIODevice>
#include <QtCore/QTextStream>
#include <QtCore/QMap>

#include <vector>

namespace Enc
{

//******************************************************************************
/// Namespace with usefull funktions/typedefs/constansts to parse/write ISO8211 Files
/*!
* REM: Several functions are plattform dependent because of byte-order, but:
*      Until now, only x86 Byte-order is implemented 
***************************************************************************** */
namespace ISO8211
{
    typedef unsigned long long LongNAMe;  ///must be a 64 Bit type, not a real int, holds content of FOID, used as handle
    typedef unsigned long long RecNAME;   ///must be a 40 Bit type, at least - holds cell-unique concat. of RCNM and RCID, used as handle

    //Unit and Field Terminators - for 8 Byte and 16 Byte character sets:
    const char UT = 0x1F;    //unit terminator - terminates variable-length subfields
    const char FT = 0x1E;    //field terminator - terminates fields in directory and field area
    const wchar_t UTL2 = 0x001F;  //unit terminator for lexLevel2 - terminates variable-length subfields
    const wchar_t FTL2 = 0x001E;  //field terminator for lexLevel2- terminates fields in directory and field area
    const char    DelChar = 0x7F;     //Attribute Delete-Character for LexLevel 0 and 1 - REM: the DEL-char is a valid ASCII-char
    const wchar_t DelChar2 = 0x007F;  //Attribute Delete-Character for LexLevel 2 

    const unsigned short RCNM_DS = 10;  //for DSID
    const unsigned short RCNM_DP = 20;  //for DSPM
    const unsigned short RCNM_FE = 100; //Feature
    const unsigned short RCNM_VI = 110; //Isolated Node (or Sounding cluster)
    const unsigned short RCNM_VC = 120; //Conncected (Bounding) Node
    const unsigned short RCNM_VE = 130; //Edge
    const unsigned short RCNM_VF = 140; //Face: not used in S-57 ENCs

    struct DirEntry
    {
        char tag[4];
        long fieldLen;
        long fieldPos;
        DirEntry() : fieldLen(0), fieldPos(0) {tag[0] =0; tag[3] =0;}
    };

    int           decDigits4Number(long long num);
    QString       number2String(long long num, int digits);

    unsigned long str2uInt(const char * startPtr, int len);
    int           findCharOffset(const char * startPos, char findMe, const char * end);
    int           findCharOffset(const char * startPos, wchar_t findMe, const char * end);
    const char *  findCharPosition(const char * startPos, char findMe, const char * end);

    bool checkDDRleader(const char * iso8211file, unsigned int fileSize, 
                               unsigned int * DDRlen = 0, unsigned int * DDRfieldStart = 0, unsigned int * sizeofFieldLen = 0, unsigned int * sizeofFieldPos = 0);
    bool   parseLeader(const char * recPtr, unsigned int * reclen, unsigned int * recFieldStart = 0, unsigned int * sizeofFieldLen = 0, unsigned int * sizeofFieldPos = 0);
    void   parseDirectory(std::vector< DirEntry > & dirEntries, const char * ptr, int len, unsigned int sizeofFieldLen, unsigned int sizeofFieldPos);

    void   writeDDRleaderS57(QTextStream & iso8211strm, unsigned int reclen, unsigned int recFieldStart, unsigned int sizeofFieldLen, unsigned int sizeofFieldPos);
    void   writeRecLeaderS57(QTextStream & iso8211strm, unsigned int reclen, unsigned int recFieldStart, unsigned int sizeofFieldLen, unsigned int sizeofFieldPos);

    void   writeDDR4catS57(QTextStream & iso8211strm);
    void   write0001fieldASC(QTextStream & iso8211strm, unsigned int iso8211rcid, unsigned int field001size);
    unsigned long write0001fieldBIN(QIODevice *, unsigned long currentIso8211rcid, unsigned long field001size);
    void   writeRecDir4CatS57(QTextStream & iso8211strm, unsigned int recLen0001, unsigned int recLenCATD, unsigned int sizeofFLF, unsigned int sizeofFPF);

    //****** methods usefull to parse ISO8211 ******
    //******************************************************************************
    /// Convert a binary 2byte or 4byte int values from little indian
    /*!
    * S-57 is little indian, so is Intel - nothing to do - but :
    * needs to be extended if used  for other plattforms    !!!!!!!!!!!!!!!!!!!!!!!!!
    * will work for 2-bytes ushort, only !!!!!!!!!!!!!!!!!!
    ***************************************************************************** */
    inline unsigned short bytes2ushort(const char * binPtr)
    {
        return *(reinterpret_cast<const unsigned short *>(binPtr));
    }

    inline short bytes2short(const char * binPtr)
    {
        return *(reinterpret_cast<const short *>(binPtr));
    }
    inline unsigned long bytes2ulong(const char * binPtr)
    {
        return *(reinterpret_cast<const unsigned long *>(binPtr));
    }

    inline long  bytes2long(const char * binPtr)
    {
        return *(reinterpret_cast<const long *>(binPtr));
    }

    QString parseLexLevel2(const char * binPtr, int len);
    QString parseLexLevel2(const char * binPtr);

    const char * rcnm2ascii(const char * binPtr);
    const char * prsp2ascii(const char * binPtr);
    const char * prof2ascii(const char * binPtr);

    //******************************************************************************
    /// Write a basic type to BINARY file
    /*!
    * This is platform depended! It will work for x86, only
    ***************************************************************************** */
    /**** methods to write ISO8211 ****
    void writeUChar(QIODevice * dev, unsigned char val);
    void writeUShort(QIODevice * dev, unsigned short val);
    void writeULong(QIODevice * dev, unsigned long val);
    void writeULongLong(QIODevice * dev, unsigned long long val);

    void writeShort(QIODevice * dev, short val);
    void writeLong(QIODevice * dev, long val);
    void writeLongLong(QIODevice * dev, long long val);*/
    inline long long writeUChar(QIODevice * dev, unsigned char val)
    {
        return dev->write(reinterpret_cast<const char *>(&val), 1);
    }
    inline long long writeUShort(QIODevice * dev, unsigned short val)
    {
#ifdef BlaBla
        const char * ptr = reinterpret_cast<const char *>(&val);
        dev->putChar(ptr[0]);
        dev->putChar(ptr[1]);
        return 2;
#else
        return dev->write(reinterpret_cast<const char *>(&val), 2);
#endif
    }
    inline long long writeULong(QIODevice * dev, unsigned long val)
    {
#ifdef BlaBla
        const char * ptr = reinterpret_cast<const char *>(&val);
        dev->putChar(ptr[0]);
        dev->putChar(ptr[1]);
        dev->putChar(ptr[2]);
        dev->putChar(ptr[3]);
        return 4;
#else
        return dev->write(reinterpret_cast<const char *>(&val), 4);
#endif
    }
    inline long long writeULongLong(QIODevice * dev, unsigned long long val)
    {
        return dev->write(reinterpret_cast<const char *>(&val), 8);
    } 

    inline long long writeShort(QIODevice * dev, short val)
    {
        return dev->write(reinterpret_cast<const char *>(&val), 2);
    }
    inline long long writeLong(QIODevice * dev, long val)
    {
#ifdef BlaBla
        const char * ptr = reinterpret_cast<const char *>(&val);
        dev->putChar(ptr[0]);
        dev->putChar(ptr[1]);
        dev->putChar(ptr[2]);
        dev->putChar(ptr[3]);
        return 4;
#else
        return dev->write(reinterpret_cast<const char *>(&val), 4);
#endif
    }
    inline long long writeLongLong(QIODevice * dev, long long val)
    {
        return dev->write(reinterpret_cast<const char *>(&val), 8);
    }

    const char * rcnm2ascii(unsigned char);
    const char * prsp2ascii(unsigned char);
    const char * prof2ascii(unsigned char);

    unsigned char rcnmFromAscii(const char * rcnmAscii);

    /// put short+long+short into one unsigned-long-long to make it easier to handle (no need for a struct) 
    inline LongNAMe makeLongNAMe(unsigned short AG, unsigned long FIDN, unsigned short FIDS)
    {
        LongNAMe ret = 0; 
        char * retPtr = reinterpret_cast<char *>(&ret);
        memcpy(retPtr, &AG, 2);
        memcpy(retPtr +2, &FIDN, 4);
        memcpy(retPtr +6, &FIDS, 2);
        return ret;
    }
    ///unfridle LongNAMe into its three components
    inline void unmakeLongNAMe(LongNAMe lName, unsigned short & AG, unsigned long & FIDN, unsigned short & FIDS)
    {
        const char * ptr = reinterpret_cast<const char *>(&lName);
        AG = * reinterpret_cast<const unsigned short *>(ptr);
        FIDN = * reinterpret_cast<const unsigned long *>(ptr +2);
        FIDS = * reinterpret_cast<const unsigned short *>(ptr +6);
    }
    QString makeLongNAMeASCII(LongNAMe lName);  ///Make readable string from LongName, likeL AG=12FIDN=1234,FIDS=12

    /// make a 64bit int containing rcnm and rcid 
    inline RecNAME makeRecName(unsigned char rcnm, unsigned long rcid)
    {
        RecNAME recName = rcnm;
        recName *= 0x100000000; //shift 4 bytes
        return recName += rcid;
    }
    //** make name like "FE/0000000123" usefull for log-files etc
    QString makeRecNameASCII(unsigned char rcnm, unsigned long rcid);
    QString makeRecNameASCII(RecNAME recName);
}
namespace S57
{
    //** DSID Subfields to String **
    QString EXPP2string(int expp);
    QString PROF2string(int prof);
    QString INTU2string(int intu);
    QString PRSP2string(int prsp);

    QString IMPL2string(QString impl);
}

//******************************************************************************
/// Simple read-only-iterator to iterate over recoreds of an ISO8211 file
/*!
* Does not care about Fields/Subfields
***************************************************************************** */
class Iso8211recordIterator
{
public:

    Iso8211recordIterator();
    ~Iso8211recordIterator();
    void init(const char * _startPtr, unsigned int _size);
    unsigned int recCount();
    const char * current(int * recSize =0) const;
    const char * getDDR(int * recSize =0) const;
    const char * at(unsigned int recInd, int * recSize =0);
    void next();
    void rewind();

protected:

    const char * startPtr;   //pointer to start of iso8211 file in mem -> points to DDR (0 means: not initializied!)
    unsigned int totalSize;  //size of iso8211 file (or a part of it if not full read)
    const char * recPtr;     //pointer to start of current record
    unsigned int recSize;    //record Size of current pointer 
    unsigned int ddrRecLen;  //length of DDR, the first record

    std::vector<const char *> recVec; //holds the start of every read Record (except DDR)
};

//******************************************************************************
/// Reads Record Direcotry to Iterate over the Fields of a ISO8211 Records
/*!
* It is a direct access iterator and a forward iterator at the same time!
***************************************************************************** */
class Iso8211fieldIterator
{
public:
    Iso8211fieldIterator(const char * _RecordStartPtr);
    ~Iso8211fieldIterator();

    unsigned long checkLength() const;
    unsigned long getRecordLength() const {return recSize;}
    unsigned int fieldCount() const {return dirEntries.size();}

    //** direct access **
    const char * at(unsigned int fieldInd, int * recSize, const char *& tag);
    const char * operator[](const char * fieldTag);
    const char * operator[](unsigned int fieldInd);
    //** sequential access **
    const char * current(int * recSize, const char *& tag) {return at(currentIndex, recSize, tag);}
    unsigned int getCurrentIndex() const {return currentIndex;}
    bool next() 
    {
        if (currentIndex >= dirEntries.size()) return false;
        ++currentIndex;
        return currentIndex < dirEntries.size() ;
    }
    void rewind(){currentIndex = 0;}

private:
    std::vector< ISO8211::DirEntry > dirEntries;
    const char * fieldStartPtr;
    unsigned int fieldStartOffset, recSize; //taken from Leader
    unsigned int currentIndex; 
};

}
#endif