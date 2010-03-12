#ifndef CELL_PARSER_ISO8211_DIRTY
#define CELL_PARSER_ISO8211_DIRTY

#include "cell_s57.h"
#include "iso8211_simple.h"
#include "cell_parser_iso8211.h"

#include <QtCore/QString>

namespace Enc
{

//*****************************************************************************
/// Simplified Class to parse a complete ISO-8211 fiele containing a S-57 ENC
/*!
* Since the S-57 ENCs only use a small subset of ISO8211, this class is simplified,
* but cannot parse other files than S-57 Chart Files
*
* This is a abstract base class:
* It only provides methods to parse Records and Fields, which can be used by 
* subclasses to parse different S-57 Cells Types 
****************************************************************************** */
class CellParser8211Dirty : public CellParser8211
{
public:

    CellParser8211Dirty(unsigned long parseOptions, double factorXY =0, double factorZ =0);
    virtual ~CellParser8211Dirty();

protected:

     void parseInit(QString cellName, CellS57_Header * cell, uint bytes2read =0);
     virtual void parseS57CellIntern(CellS57_Header * cell);

    //**** Methods ****
    void readDDR();

    void parseDSIDField(const char * fieldPtr, int fieldLen, FieldDSID & dsid);
    void parseDSSIField(const char * fieldPtr, int fieldLen, FieldDSSI & dssi);
    void parseDSPMField(const char * fieldPtr, int fieldLen, FieldDSPM & dspm);

    //** Feature Record Field Parser **
    virtual void parseFeatureRecord(Iso8211fieldIterator & fieldIt) =0;
    void parseFeatureRecord(Iso8211fieldIterator & fieldIt, FeatureS57 & feat);
    void parseFeatureUpdtRecord(Iso8211fieldIterator & fieldIt, FeatureS57_Updt & feat);
    bool parseFeatureField(int &fieldSize, const char * tag, const char * field, FeatureS57 & feat); 
    void parseFRIDField(const char * fieldPtr, int fieldLen, FieldFRID & frid);
    void parseFOIDField(const char * fieldPtr, int fieldLen, FieldFOID & foid);
    long parseATTF_NAFTField(const char * fieldPtr, int fieldLen, int lexLevel, bool isNat, FeatureS57 & feat);
    void parseFFPCField(const char * fieldPtr, int fieldLen, FieldFFPC & ffpc); //UPDATES only
    long parseFFPTField(const char * fieldPtr, int fieldLen, FeatureS57 & feat);
    void parseFSPCField(const char * fieldPtr, int fieldLen, FieldFSPC & fspc); //UPDATES only
    long parseFSPTField(const char * fieldPtr, int fieldLen, FeatureS57 & feat);
    
    //** Vector Record Field Parser **
    virtual void parseVectorRecord(Iso8211fieldIterator & fieldIt) =0;
    void parseVRIDField(const char * fieldPtr, int fieldLen, FieldVRID & vrid);
    void parseEdgeRecord(Iso8211fieldIterator & fieldIt, EdgeS57 & edgeRec);
    void parseNodeRecord(Iso8211fieldIterator & fieldIt, NodeS57 & nodeRec);
    void parseSndgRecord(Iso8211fieldIterator & fieldIt, SoundgS57 & sndgRec);
    void parseATTVFields(const char * fieldPtr, int fieldLen, SpatialS57 & spRec);
    void parseVRPCField(const char * fieldPtr, int fieldLen, FieldVRPC & vrpc); //UPDATES only
    void parseVRPTField(const char * fieldPtr, int fieldLen, EdgeS57 & edgeRec);
    void parseSGCCField(const char * fieldPtr, int fieldLen, FieldSGCC & sgcc); //UPDATES only
    long parseSG2DField(const char * fieldPtr, int fieldLen, double &, double &);
    void parseSG2DFields(const char * fieldPtr, int fieldLen, std::vector< double > & );
    void parseSG3DFields(const char * fieldPtr, int fieldLen, std::vector< double > &);



    //**** Data ****

    char * fileBuffer; //Buffer to store the iso8211-file (or part of it)
    uint fileBufSize;   //size of fileBuffer
    uint fileSize;      //size of fileBuffer really used

    double facXY; //COMF from Cellheader, but as double
    double facZ;  //SOMF from Cellheader, but as double

    unsigned char AALL, NALL; //Lex. Level vor Attributes/Nat.Attributes

    //** just for information: count supicious fields while parsing cell - should be ZERO if cell is ok**
    int unusedVRPT; //VRPT not needed for Edge->BouningNode  
    int unusedSpatTag; //Other unused (non-standard) subfields in Spatials
    int unusedFeatTag;  //Other unused (non-standard) subfields in Spatials
};


}

#endif