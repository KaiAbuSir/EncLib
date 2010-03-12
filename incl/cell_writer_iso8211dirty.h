#ifndef CELL_WRITER_ISO8211_DIRTY
#define CELL_WRITER_ISO8211_DIRTY

#include <QtCore/QIODevice>

#include "cell_writer.h"
#include "cell_records.h"
namespace Enc
{
    
//*****************************************************************************
/// Write a BaseCell to ISO8211 File
/*!
****************************************************************************** */
class CellWriter8211Dirty : public CellWriter
{
public:

    using CellWriter::writeS57Cell;

    CellWriter8211Dirty(const CellS57_Base * cell=0) : CellWriter(cell), cellDev(0), field0001len(0), currentIso8211rcid(0) {}
    virtual ~CellWriter8211Dirty();
    virtual void writeS57Cell(QIODevice * device2write);

protected:

    //**** methods ****

    //** Records **
    unsigned long writeDSGIrecord();  //CellHeader
    unsigned long writeDSGRrecord();  //CellHeader
    unsigned long writeNode(const NodeS57 & node);
    unsigned long writeSounding(const SoundgS57 & sndg);
    unsigned long writeEdge(const EdgeS57 & edge);
    unsigned long writeFeature(const FeatureS57 & feat);

    unsigned long writeSpatial(const SpatialS57 & spatial, const char * coordTag, unsigned long coordSize, unsigned long vrptSize = 0);

    //** common Fields **
    unsigned long write0001Field();
    //** cellheader Fields **
    unsigned long writeDSIDField();
    unsigned long writeDSSIField();
    unsigned long writeDSPMField();
    //** feature Record Fields **
    unsigned long writeFridField(const FieldFRID &);
    unsigned long writeFoidField(const FieldFOID &);
    unsigned long writeAttributeField(const FieldAttr &);
    unsigned long writeFfpcField(const FieldFFPC &);
    unsigned long writeFfptField(const FieldFFPT &);
    unsigned long writeFspcField(const FieldFSPC &);
    unsigned long writeFsptField(const FieldFSPT &);

    //** Spatial Record Fields **
    unsigned long writeVridField(const FieldVRID &);
    unsigned long writeVrpcField(const FieldVRPC &);
    unsigned long writeVrptField(const FieldVRPT &);
    unsigned long writeSGCCField(const FieldSGCC &);

    void makeVRPT4BoundingNode(FieldVRPT & vrpt, unsigned long recId, bool forward) const;

    //**** data ****
    QIODevice * cellDev;
    const static char * DefaultDDR;

    int field0001len; //size of 0001 Field   EXCLUDING the FT !! (ISO8211 RecId)- must be 2 bytes (DDR) but: depending  on Record count, may be 4 bytes!
    unsigned long currentIso8211rcid; //to be updated AFTER the 0001 Field is written
};

//*****************************************************************************
/// 
/*!
****************************************************************************** */
class RecordLeaderDirWriter
{
public:
#ifdef _DEBUG
    unsigned int recLenWrittn;
#endif

    RecordLeaderDirWriter(int total0001Fieldlength,int sizeofFieldLenField = 0, int sizeofFieldPosField = 0) 
         : total0001Fieldlen(total0001Fieldlength), sizeFldLenFld(sizeofFieldLenField), sizeFldPosFld(sizeofFieldPosField) {}
    unsigned long write(QIODevice *) const;
    void addField(const char * tag, int lenght);
    
private:
    std::vector< ISO8211::DirEntry > dirEntries;
    int total0001Fieldlen;
    int sizeFldLenFld, sizeFldPosFld;

};

}

#endif