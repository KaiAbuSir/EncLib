#ifndef CELL_RECORDS_FIELDS_H
#define CELL_RECORDS_FIELDS_H

#include <QtCore/QString>
#include <QtCore/QVariant>

#include <memory.h>
#include <vector>
#include <map>
#include <algorithm>

#include "iso8211_simple.h"

namespace Enc
{
using namespace ISO8211;

//******************************************************************************
/// Struct containig the data (subfields) of the DSID Field (=S-57 cell header, first field)
/*!
* found in S-57 cell header record: first record, first field
***************************************************************************** */
class FieldDSID
{
public:

    const static unsigned int MemberCnt = 16;  //number of data-members in struct
    enum ColumnNames{COL_RCNM =0, COL_RCID, COL_EXPP, COL_INTU, COL_DSNM, COL_EDTN , COL_UPDN , COL_UADT , COL_ISDT , 
                     COL_STED, COL_PRSP, COL_PSDN, COL_PRED, COL_PROF, COL_AGEN, COL_COMT};
    QVariant operator[](int index)const;
    FieldDSID & operator=(const FieldDSID &);  
    bool operator==(const FieldDSID &) const;

    //** Methods **
    FieldDSID();
    FieldDSID(const FieldDSID &);
    void clear() {*this = FieldDSID();}
    unsigned long getIso8211Length() const;
 
    unsigned char  getRCNM() const {return RCNM;}  //Recored Name always "DS" - unimportant
    unsigned long  getRCID() const {return RCID;}  //Record ID - unimportant
    unsigned char  getEXPP() const {return EXPP;}  //Exchange purpose 1 or 2 - means: 'N' (new) or 'R' (revision)
    unsigned char  getINTU() const {return INTU;}  //Intended usage 
    QString        getDSNM() const {return DSNM;}  //Data Set Name
    int            getEDTN() const {return EDTN;}  //Edition Number
    int            getUPDN() const {return UPDN;}  //Update Number
    const char *   getUADT() const {return UADT;}  //Update application Date
    const char *   getISDT() const {return ISDT;}  //Issue Date
    const char *   getSTED() const {return STED;}  //Edition Number S-57 always "03.1" - unimportant
    unsigned char  getPRSP() const {return PRSP;}  //Product Specification - normally 1 = "ENC" 
    QString        getPSDN() const {return PSDN;}  //Product Specification description - unimportant
    QString        getPRED() const {return PRED;}  //Product Specification edition number - unimportant
    unsigned char  getPROF() const {return PROF;}  //Application profile identification - "EN"(1) "ER"(2) or "DD"(3)
    unsigned short getAGEN() const {return AGEN;}  //Producing agency id (because it is stored binary in cell)
    QString        getCOMT() const {return COMT;}  //Comment

    void setRCNM(const char * val)   {setRCNM(ISO8211::rcnmFromAscii(val));}  //Recored Name always "DS" - unimportant
    void setRCNM(unsigned char val)  {RCNM= val;} //Record Name - set number-value directly
    void setRCID(unsigned long val)  {RCID = val;}  //Record ID - unimportant
    void setEXPP(unsigned char val)  {EXPP = val;}  //Exchange purpose 1 or 2 - means: 'N' (new) or 'R' (revision)
    void setINTU(unsigned char val)  {INTU = val;}  //Intended usage 
    void setDSNM(QString val)        {DSNM = val;}  //Data Set Name
    void setEDTN(int val)            {EDTN = val;}  //Edition Number
    void setUPDN(int val)            {UPDN = val;}  //Update Number
    void setUADT(const char * val)   {strncpy(UADT,val, 8);UADT[8]=0;}  //Update application Date
    void setISDT(const char * val)   {strncpy(ISDT,val, 8);ISDT[8]=0;}  //Issue Date
    void setSTED(const char * val)   {strncpy(STED,val, 4);STED[4]=0;}  //Edition Number S-57 always "03.1" - unimportant
    void setPRSP(unsigned char val)  {PRSP = val;}  //Product Specification - normally 1 = "ENC" 
    void setPSDN(QString val)        {PSDN = val;}  //Product Specification description - unimportant
    void setPRED(QString val)        {PRED = val;}  //Product Specification edition number - unimportant
    void setPROF(unsigned char val)  {PROF = val;}  //Application profile identification - "EN"(1) "ER"(2) or "DD"(3)
    void setAGEN(unsigned short val) {AGEN = val;}  //Producing agency id (because it is stored binary in cell)
    void setCOMT(QString val)        {COMT = val;}  //Comment

private:

    //** The Cell Header Data (Record Subfields) **
    unsigned long  RCID;  //Record ID - unimportant
    unsigned char  RCNM;  //Recored Name always 10 = "DS" - unimportant
    unsigned char  EXPP;  //Exchange purpose 1 or 2 - means: 'N' (new) or 'R' (revision)
    unsigned char  INTU;  //Intended usage 
    unsigned char  PROF;  //Application profile identification - "EN"(1) "ER"(2) or "DD"(3)
    unsigned char  PRSP;  //Product Specification - normally 1 = "ENC" 
    QString        DSNM;  //Data Set Name
    long           EDTN;  //Edition Number
    long           UPDN;  //Update Number
    char           UADT[9];  //Update application Date, MUST be 8 chars long!
    char           ISDT[9];  //Issue Date, MUST be 8 chars long!
    char           STED[5];  //Edition Number S-57 always "03.1" - unimportant, but MUST be 4 chars long!
    QString        PSDN;  //Product Specification description - unimportant
    QString        PRED;  //Product Specification edition number - unimportant
    QString        COMT;  //Comment
    unsigned short AGEN;  //Producing agency id (because it is stored binary in cell)
};

//******************************************************************************
/// Struct containig the data (subfields) of the DSSI Field 
/*! 
* found in S-57 cell header record: first record, second field
* Contains mainly record counts (unimportant), but: LEXICAL LEVELS are important!
***************************************************************************** */
class FieldDSSI
{
public:
    const static unsigned int MemberCnt = 11;  //number of data-members in struct
    enum ColumnNames{COL_DSTR=0, COL_AALL, COL_NALL, COL_NOMR, COL_NOCR, COL_NOGR, COL_NOLR, COL_NOIN, COL_NOCN, COL_NOED, COL_NOFA};
    QVariant operator[](int index)const;
    FieldDSSI();
    void clear() {*this = FieldDSSI();}
    unsigned long getIso8211Length() const;

    unsigned char getDSTR() const {return DSTR;} //Topology: should be 3(PG = Planar Graph) or 0 (NO = Not Relevant)
    unsigned char getAALL() const {return AALL;}  //Lexical level of ATTF fields - may be 0 or 1
    unsigned char getNALL() const {return NALL;}  //Lexical level of NATF fields - may be 0,1 or 2
    //reocord-counter subfields:
    long           getNOMR() const {return NOMR;} //meta records
    long           getNOCR() const {return NOCR;} //cartographic records - sould be 0 !!!
    long           getNOGR() const {return NOGR;} //number of geo records
    long           getNOLR() const {return NOLR;} //number of collection records
    long           getNOIN() const {return NOIN;} //number of isolated nodes
    long           getNOCN() const {return NOCN;}
    long           getNOED() const {return NOED;}
    long           getNOFA() const {return NOFA;}

     void setDSTR(unsigned char val)  { DSTR = val;} //Topology: should be 3(PG = Planar Graph) or 0 (NO = Not Relevant)
     void setAALL(unsigned char val)  { AALL = val;}  //Lexical level of ATTF fields - may be 0 or 1
     void setNALL(unsigned char val)  { NALL = val;}  //Lexical level of NATF fields - may be 0,1 or 2
    //reocord-counter subfields:
    void setNOMR(long val)  { NOMR = val;} //meta records
    void setNOCR(long val)  { NOCR = val;} //cartographic records - sould be 0 !!!
    void setNOGR(long val)  { NOGR = val;} //number of geo records
    void setNOLR(long val)  { NOLR = val;} //number of collection records
    void setNOIN(long val)  { NOIN = val;} //number of isolated nodes
    void setNOCN(long val)  { NOCN = val;}
    void setNOED(long val)  { NOED = val;}
    void setNOFA(long val)  { NOFA = val;}

private:
    unsigned char DSTR; //Topology: should be 3(PG = Planar Graph) or 0 (NO = Not Relevant)
    unsigned char AALL;  //Lexical level of ATTF fields - may be 0 or 1
    unsigned char NALL;  //Lexical level of NATF fields - may be 0,1 or 2
    //reocord-counter subfields:
    long           NOMR; //meta records
    long           NOCR; //cartographic records - sould be 0 !!!
    long           NOGR; //number of geo records
    long           NOLR; //number of collection records
    long           NOIN; //number of isolated nodes
    long           NOCN;
    long           NOED;
    long           NOFA;
};

//******************************************************************************
/// Struct containig the data (subfields) of the DSPM Field 
/*! 
* found in S-57 cell header record: second record, first field
***************************************************************************** */
class FieldDSPM
{
public:
    const static unsigned int  MemberCnt = 13;  //number of members in CatalogEntry-struct: 10
    enum ColumnNames{COL_RCNM=0, COL_RCID , COL_HDAT , COL_VDAT , COL_SDAT , COL_CSCL , COL_DUNI , COL_HUNI , COL_PUNI , COL_COUN , COL_COMF , COL_SOMF, COL_COMT};
    QVariant operator[](int index)const;
    FieldDSPM();
    void clear() {*this = FieldDSPM();}
    unsigned long getIso8211Length() const;

    unsigned char getRCNM() const {return RCNM;} //Record Name - always "DP" 
    unsigned long getRCID() const {return RCID;}    //Record Identification Number: Ranges: 1 to 2^32-2
    int           getHDAT() const {return HDAT;}    //Horizontal Datum - in S-57 always WGS84
    int           getVDAT() const {return VDAT;}    //Vertical Datum
    int           getSDAT() const {return SDAT;}    //Sounding Datum
    unsigned long getCSCL() const {return CSCL;}    //Compilation Scale
    int           getDUNI() const {return DUNI;}    
    int           getHUNI() const {return HUNI;}
    int           getPUNI() const {return PUNI;}
    int           getCOUN() const {return COUN;} //Coordiante Units - always 1 = "LL" (Lat-Lon) for S-57 ENCs
    unsigned long getCOMF() const {return COMF;} //Coordinate Multiplication Factor
    unsigned long getSOMF() const {return SOMF;} // 3D (Sounding) Multiplication Factor
    QString       getCOMT() const {return COMT;} //Comment

    void setRCNM(const char * val)  {setRCNM(ISO8211::rcnmFromAscii(val));} //Record Name - always "DP" 
    void setRCNM(unsigned char val) {RCNM= val;} //Record Name - always "DP" 
    void setRCID(unsigned long val) {RCID= val;}    //Record Identification Number: Ranges: 1 to 2^32-2
    void setHDAT(int val)           {HDAT= val;}    //Horizontal Datum - in S-57 always WGS84
    void setVDAT(int val)           {VDAT= val;}    //Vertical Datum
    void setSDAT(int val)           {SDAT= val;}    //Sounding Datum
    void setCSCL(unsigned long val) {CSCL= val;}    //Compilation Scale
    void setDUNI(int val)           {DUNI= val;}    
    void setHUNI(int val)           {HUNI= val;}
    void setPUNI(int val)           {PUNI= val;}
    void setCOUN(int val)           {COUN= val;} //Coordiante Units - always 1 = "LL" (Lat-Lon) for S-57 ENCs
    void setCOMF(unsigned long val) {COMF= val;} //Coordinate Multiplication Factor
    void setSOMF(unsigned long val) {SOMF= val;} // 3D (Sounding) Multiplication Factor
    void setCOMT(QString val)       {COMT= val;} //Comment

private:
    unsigned char RCNM; //Record Name - always 20 = "DP" 
    unsigned long RCID; //Record Identification Number: Ranges: 1 to 2^32-2
    int           HDAT; //Horizontal Datum - in S-57 always WGS84
    int           VDAT; //Vertical Datum
    int           SDAT; //Sounding Datum
    unsigned long CSCL; //Compilation Scale
    int           DUNI;    
    int           HUNI;
    int           PUNI;
    int           COUN; //Coordiante Units - always 1 = "LL" (Lat-Lon) for S-57 ENCs
    unsigned long COMF; //Coordinate Multiplication Factor
    unsigned long SOMF; // 3D (Sounding) Multiplication Factor
    QString       COMT; //Comment
};


//******************************************************************************
/// Struct containig the data (subfields) of the FRID Field 
/*!
* found in S-57 Feature Records
***************************************************************************** */
class FieldFRID
{
friend class CellParser8211;
public:
    const static unsigned int MemberCnt = 7;  

    //** Methods **
    FieldFRID():  PRIM(0), GRUP(0), RUIN(0), RCNM(0), RCID(0), OBJL(0), RVER(0) {}
    unsigned long getIso8211Length() const {return 13;}

    unsigned char  getPRIM() const {return PRIM;}    //geo.Primitive: 1,2,3,255 = Point,Line,Area or NULL
    unsigned char  getGRUP() const {return GRUP;}    //Group: 1,2, or 255
    unsigned char  getRUIN() const {return RUIN;}    //1,2 or 3 = I,D or M
    unsigned char  getRCNM() const {return RCNM;}    //Always 100 = "FE" for Feature Records
    unsigned long  getRCID() const {return RCID;}    //from 1 to 2^32-2
    unsigned short getOBJL() const {return OBJL;}    //Object label
    unsigned short getRVER() const {return RVER;}    //Record Version Serial Number

    void setPRIM(unsigned char val)  {PRIM = val  ;}   
    void setGRUP(unsigned char val)  {GRUP = val  ;}   
    void setRUIN(unsigned char val)  {RUIN = val  ;}    
    void setRCNM(unsigned char val)  {RCNM = val  ;}   
    void setRCID(unsigned long val)  {RCID = val  ;}    
    void setOBJL(unsigned short val) {OBJL = val  ;}    
    void setRVER(unsigned short val) {RVER = val  ;}    

private:
    //** Data **
    unsigned char  PRIM;    //geo.Primitive: 1,2,3,255 = P,L,N or NULL
    unsigned char  GRUP;    //Group: 1,2, or 255
    unsigned char  RUIN;    //1,2 or 3 = I,D or M
    unsigned char  RCNM;    //Always 100 = "FE" for Feature Records
    unsigned long  RCID;    //from 1 to 2^32-2
    unsigned short OBJL;    //Object label
    unsigned short RVER;    //Record Version Serial Number
};

//******************************************************************************
/// Struct containig the data (subfields) of the Feature Object ID (FOID)
/*!
* found in S-57 Feature Records
***************************************************************************** */
class FieldFOID
{
friend class CellParser8211;
public:

    //** Methods:
    FieldFOID() : AGEN(0), FIDS(0), FIDN(0)  {}
    unsigned long getIso8211Length() const {return 8+1;}

    unsigned short  getAGEN() const {return AGEN;}  //Agency code 
    unsigned short  getFIDS() const {return FIDS;}  //1 to 2^16 -2
    unsigned long   getFIDN() const {return FIDN;}  //1 to 2^32 -2

    void setAGEN(unsigned short val) {AGEN = val;}  //Agency code 
    void setFIDS(unsigned short val) {FIDS = val;}  //1 to 2^16 -2
    void setFIDN(unsigned long  val) {FIDN = val;}  //1 to 2^32 -2

    inline LongNAMe getLongNAMe() const { return makeLongNAMe(AGEN, FIDN, FIDS);}

private:
    //** Data **
    unsigned short  AGEN;  //Agency code 
    unsigned short  FIDS;  //1 to 2^16 -2
    unsigned long   FIDN;  //1 to 2^32 -2

};

//******************************************************************************
/// Struct containig the data (subfields) of an Attribute Field (may be ATTF or NATF)
/*!
* found in S-57 Feature Records, and there may be many of them in a Feature
***************************************************************************** */
class FieldAttr
{
public:

    FieldAttr() : ATTL(0), isNat(false) {}
    unsigned long getIso8211Length(bool lexLevel2) const  //length WITHOUT final FT
    {
        unsigned long fieldLen = 2;  //ATTL 
        if (lexLevel2) fieldLen += 2* value.length() +2; //stringLen +UT
        else           fieldLen +=  value.length() +1;   //stringLen + UT
        return fieldLen;
    }

    unsigned short  getATTL() const  {return  ATTL;}     //16 bit Attribte Code
    bool            getNat()const    {return  isNat;}    
    QString         getValue() const {return  value;}    //Attribute Value

    void setATTL(unsigned short val)  {ATTL = val;}     //Attribte Code
    void setNat(bool nat)             {isNat = nat;}
    void setValue(QString val)        {value = val;}    //Attribute Value

private:
    unsigned short  ATTL;     //Attribte Code
    bool            isNat;    //if true-> nationa attribute (NATF)
    QString         value;    //Attribute Value, may be ASCII, LATIN! or 16bit unicode
};

//******************************************************************************
/// Struct containig the update-instructions for the Feature to Feature Pointer
/*!
* 
***************************************************************************** */
class FieldFFPC
{
public:
    FieldFFPC() : FFUI(0), FFIX(0), NFPT(0) {}
    unsigned long getIso8211Length() const {return 5 +1;}

    unsigned char  getFFUI() const {return FFUI;} //1,2,3 = Insert, Delete Modify
    unsigned short getFFIX() const {return FFIX;}
    unsigned short getNFPT() const {return NFPT;}

    void setFFUI(unsigned char val)  { FFUI = val;} //1,2,3 = Insert, Delete Modify
    void setFFIX(unsigned short val)  { FFIX = val;}
    void setNFPT(unsigned short val)  { NFPT = val;}

private:
    unsigned char FFUI; //1,2,3 = Insert, Delete Modify
    unsigned short FFIX;
    unsigned short NFPT;
};

//******************************************************************************
/// Struct containig the data (subfields) of the Feature to Feature Pointer
/*!
* 
***************************************************************************** */
class FieldFFPT
{
public:

    FieldFFPT(): LNAM(0), RIND(0) {}
    unsigned long getIso8211Length() const {return 8 +1 +COMT.length() +1;}  //Including UT, WITHOUT FT

    unsigned long long getLNAM() const {return LNAM;} //64 bit field: Actually not a long long ,but: Concatination:  AGEN + FIDN + FIDS of FOID-field
    unsigned char      getRIND() const {return RIND;} //1,2 or 3 = Master, Slave or Peer (M,S,P)
    QString            getCOMT() const {return COMT;} //comment

    void setLNAM(unsigned long long val) { LNAM = val;} //64 bit field: Actually not a long long ,but: Concatination:  AGEN + FIDN + FIDS of FOID-field
    void setRIND(unsigned char val)      { RIND = val;} //1,2 or 3 = Master, Slave or Peer (M,S,P)
    void setCOMT(QString val)            { COMT = val;} //comment

private:
    unsigned long long LNAM; //64 bit field: Actually not a long long ,but: Concatination:  AGEN + FIDN + FIDS of FOID-field
    unsigned char RIND; //1,2 or 3 = Master, Slave or Peer (M,S,P)
    QString COMT; //comment
};

//******************************************************************************
/// Struct containig the data (subfields) of the Feature to Spatial Pointer
/*!
* 
***************************************************************************** */
class FieldFSPC
{
public:
    FieldFSPC() : FSUI(0), FSIX(0), NSPT(0) {}
    unsigned long getIso8211Length() const {return 5 +1;}

    unsigned char  getFSUI() const {return FSUI;} //1,2,3 = Insert, Delete Modify
    unsigned short getFSIX() const {return FSIX;}
    unsigned short getNSPT() const {return NSPT;}

    void setFSUI(unsigned char val)   {FSUI = val;} //1,2,3 = Insert, Delete Modify
    void setFSIX(unsigned short val)  {FSIX = val;}
    void setNSPT(unsigned short val)  {NSPT = val;}

private:
    unsigned char  FSUI; //1,2,3 = Insert, Delete Modify
    unsigned short FSIX;
    unsigned short NSPT;
};

class FieldFSPT
{
public:
    FieldFSPT() : otherRCID(0), otherRCNM(0), ORNT(0), USAG(0), MASK(0) {;}
    unsigned long getIso8211Length() const {return 8;}  //WITHOUT FT

    RecNAME getOtherRecName() const {return makeRecName(otherRCNM, otherRCID);}
    unsigned long getOtherRCID() const {return otherRCID;} //Rest of NAME
    unsigned char getOtherRCNM() const {return otherRCNM;} //First Byte of NAME
    unsigned char getORNT() const {return ORNT;}    //Orientation: 1,2,255 = Forward, Reverse, NULL 
    unsigned char getUSAG() const {return USAG;}    //Usage Indicator: 1,2,3, 255 = Ext., Int., DataLimit, NULL
    unsigned char getMASK() const {return MASK;}    //Masking: 1,2,255 = Mask, Show, NULL

    void setOtherRCID(unsigned long val)  {otherRCID = val;} //Rest of NAME
    void setOtherRCNM(unsigned char val)  {otherRCNM = val;} //First Byte of NAME
    void setORNT(unsigned char val)  {ORNT = val;}    //Orientation: 1,2,255 = Forward, Reverse, NULL 
    void setUSAG(unsigned char val)  {USAG = val;}    //Usage Indicator: 1,2,3, 255 = Ext., Int., DataLimit, NULL
    void setMASK(unsigned char val)  {MASK = val;}    //Masking: 1,2,255 = Mask, Show, NULL

private:
    //unsigned char NAME[5]; //Pointer to Spatial = RCNM + RCID (1+4Byte)
    unsigned long otherRCID; //Rest of NAME
    unsigned char otherRCNM; //First Byte of NAME

    unsigned char ORNT;    //Orientation: 1,2,255 = Forward, Reverse, NULL 
    unsigned char USAG;    //Usage Indicator: 1,2,3, 255 = Ext., Int., DataLimit, NULL
    unsigned char MASK;    //Masking: 1,2,255 = Mask, Show, NULL
};


//******************************************************************************
//************************   Spatial Record Fields   ***************************
//******************************************************************************

//******************************************************************************
/// Struct containig the data (subfields) of the Vector Record IDentifyer
/*!
* 
***************************************************************************** */
class FieldVRID
{
public:
    //** Methods **
    FieldVRID(): RCNM(0), RUIN(0), RVER(0), RCID(0) {}
    unsigned long getIso8211Length() const {return 8 +1;}

    unsigned char  getRCNM() const {return RCNM;}    //Might be 110,120,130,140 = VI,VC,VE,VF - VF(Face) is forbidden 
    unsigned char  getRUIN() const {return RUIN;}    //1,2 or 3 = I,D or M
    unsigned short getRVER() const {return RVER;}    //Record Version Serial Number
    unsigned long  getRCID() const {return RCID;}    //from 1 to 2^32-2

    void setRCNM(unsigned char val)   { RCNM = val;}    //Might be 110,120,130,140 = VI,VC,VE,VF - VF(Face) is forbidden 
    void setRUIN(unsigned char val)   { RUIN = val;}    //1,2 or 3 = I,D or M
    void setRVER(unsigned short val)  { RVER = val;}    //Record Version Serial Number
    void setRCID(unsigned long val)   { RCID = val;}    //from 1 to 2^32-2

private:
    //** Data **
    unsigned char  RCNM;    //Might be 110,120,130,140 = VI,VC,VE,VF - VF(Face) is forbidden 
    unsigned char  RUIN;    //1,2 or 3 = I,D or M
    unsigned short RVER;    //Record Version Serial Number
    unsigned long  RCID;    //from 1 to 2^32-2
};


//******************************************************************************
/// Vector Record Pointer Control Field - ONLY 4 UPDATES
/*!
* 
***************************************************************************** */
class FieldVRPC
{
public:
    //** Methods **
    FieldVRPC() : VPUI(0), VPIX(0), NVPT(0) {}
    unsigned long getIso8211Length() const {return 5 +1;}  //INCLUDING FT

    unsigned char  getVPUI() const {return VPUI;}
    unsigned short getVPIX() const {return VPIX;}
    unsigned short getNVPT() const {return NVPT;}

    void setVPUI(unsigned char val)  {VPUI = val;}
    void setVPIX(unsigned short val) {VPIX = val;}
    void setNVPT(unsigned short val) {NVPT = val;}

private:
    //** Data: **
    unsigned char  VPUI;
    unsigned short VPIX;
    unsigned short NVPT;
};

//******************************************************************************
/// Struct containig the data (subfields) of the Vector Record Pointer Field
/*!
* THE FIELDS ARE NOT USED IN S-57 ENCs (but may be needed in other S-57 Products)
***************************************************************************** */
class FieldVRPT
{
public:
    FieldVRPT() : otherRCID(0), otherRCNM(0), ORNT(0), USAG(0), TOPI(0), MASK(0) {}
    unsigned long getIso8211Length() const {return 9;}  //WITHOUT FT, because this is a vector-field

    RecNAME getRecName() const {return makeRecName(otherRCNM, otherRCID);} 

    unsigned long getOtherRCID() const {return otherRCID;}
    unsigned char getOtherRCNM() const {return otherRCNM;}
    unsigned char getORNT() const {return ORNT;}    //Orientation: 1,2,255 = Forward, Reverse, NULL -- NOT IN ENC2.0 !!
    unsigned char getUSAG() const {return USAG;}    //Usage Indicator: 1,2,3,255 = Ext., Int., Ext.DataLimit, NULL -- NOT IN ENC2.0 !!
    unsigned char getTOPI() const {return TOPI;}    //1=Beginning Node, 2=Ending Node - Forbidden: 3,4,5,NULL -- NOT IN ENC2.0 !!
    unsigned char getMASK() const {return MASK;}    //1=Mask, 2=Show, 255=NULL -- NOT IN ENC2.0 !!

    void setOtherRCID(unsigned long val)  { otherRCID = val;}
    void setOtherRCNM(unsigned char val)  { otherRCNM = val;}
    void setORNT(unsigned char val)  { ORNT = val;}    //Orientation: 1,2,255 = Forward, Reverse, NULL -- NOT IN ENC2.0 !!
    void setUSAG(unsigned char val)  { USAG = val;}    //Usage Indicator: 1,2,3,255 = Ext., Int., Ext.DataLimit, NULL -- NOT IN ENC2.0 !!
    void setTOPI(unsigned char val)  { TOPI = val;}    //1=Beginning Node, 2=Ending Node - Forbidden: 3,4,5,NULL -- NOT IN ENC2.0 !!
    void setMASK(unsigned char val)  { MASK = val;}    //1=Mask, 2=Show, 255=NULL -- NOT IN ENC2.0 !!

private:
    unsigned long otherRCID;
    unsigned char otherRCNM;

    unsigned char ORNT;    //Orientation: 1,2,255 = Forward, Reverse, NULL -- NOT IN ENC2.0 !!
    unsigned char USAG;    //Usage Indicator: 1,2,3,255 = Ext., Int., Ext.DataLimit, NULL -- NOT IN ENC2.0 !!
    unsigned char TOPI;    //1=Beginning Node, 2=Ending Node - Forbidden: 3,4,5,NULL -- NOT IN ENC2.0 !!
    unsigned char MASK;    //1=Mask, 2=Show, 255=NULL -- NOT IN ENC2.0 !!
};


//******************************************************************************
/// Coordinate Control Field - ONLY 4 UPDATES
/*!
* 
***************************************************************************** */
class FieldSGCC
{
public:
    FieldSGCC() : CCUI(0), CCIX(0), CCNC(0) {}
    unsigned long getIso8211Length() const {return 5 +1;} //INCLUDING FT

    unsigned char  getCCUI() const {return CCUI;}
    unsigned short getCCIX() const {return CCIX;}
    unsigned short getCCNC() const {return CCNC;}

    void setCCUI(unsigned char  val) {CCUI = val;}
    void setCCIX(unsigned short val) {CCIX = val;}
    void setCCNC(unsigned short val) {CCNC = val;}

private:
    unsigned char  CCUI;
    unsigned short CCIX;
    unsigned short CCNC;
};




}
#endif

