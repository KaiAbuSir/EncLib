#ifndef CELL_S57_BASE
#define CELL_S57_BASE

#include <QtCore/QStringList>

#include "cell_check_result.h"
#include "cell_s57.h"

namespace Enc
{
class CellParser8211;
class CellParser8211Dirty;
class CellWriter;
class CellWriter8211Dirty;
class CellParser8211Dirty4Base;
class S57Updater;

//******************************************************************************
/// Class containig the data (records) of a complete S-57 cell
/*!
* Holds ALL information of a base cell, and:
* For every relation(Pointer), the class holds a backward-relation to acellerate access
***************************************************************************** */
class CellS57_Base : public CellS57_Header
{
friend CellParser8211;
friend CellParser8211Dirty;
friend CellWriter;
friend CellWriter8211Dirty;
friend CellParser8211Dirty4Base;
friend S57Updater;

public:

    //**** Methods ****
    CellS57_Base();
    virtual ~CellS57_Base();
    virtual CellS57Class getClass() const {return BaseCellClass;} //usefull for casting
    virtual void clear();

    void parseISO8211(QString fileNamePath);
    void writeISO8211(QString fileNamePath) const;
    void applyUpdates(QStringList updateCells);

    void check() const;
    int  recordCount() const;
    void updateDSSIrecordCounts();
    virtual bool isUpdateCell() const {return false;}

    const std::map< unsigned long, FeatureS57 *> &  getFeatures()  const {return features;}  //Key = RCID (RCNM is 100("FE"), anyway -> ignored)
    const std::map< unsigned long, NodeS57 *> &     getINodes()    const {return iNodes;}    //KEY: RecordId of an Isolated Node ("VI")
    const std::map< unsigned long, BoundNodeS57 *> &getBNodes()    const {return bNodes;}    //KEY: RecordId of an Bounding Node ("VC") 
    const std::map< unsigned long, SoundgS57 *> &   getSoundingds()const {return soundings;} //KEY: Rec-id of an Isolanted Node containing 3D-Cluster ("VI")
    const std::map< unsigned long, EdgeS57 *> &     getEdges()     const {return edges;}     //KEY: Rec-id of an Edge ("VE")

    bool recordExists(unsigned char RCNM, unsigned long RCID) const;
    void deleteRecordStupid(unsigned char RCNM, unsigned long RCID);
    void deleteRecordAndBackPointer(unsigned char RCNM, unsigned long RCID);
    void deleteFeatureWithOrphans(unsigned long RCID);
    void deleteSpatialWithOrphanSpatials(unsigned char RCNM, unsigned long RCID);

    FeatureS57 & getFeature(unsigned long RCID);
    SpatialS57 & getSpatial(unsigned int rcnm, unsigned long rcid);
    NodeS57 & getINode(unsigned long RCID);
    BoundNodeS57 & getBNode(unsigned long RCID);
    SoundgS57 & getSounding(unsigned long RCID);
    EdgeS57 & getEdge(unsigned long RCID);

    const FeatureS57 & getFeature(unsigned long RCID) const;
    const SpatialS57 & getSpatial(unsigned int rcnm, unsigned long rcid) const;
    const NodeS57 & getINode(unsigned long RCID) const;
    const BoundNodeS57 & getBNode(unsigned long RCID) const;
    const SoundgS57 & getSounding(unsigned long RCID) const;
    const EdgeS57 & getEdge(unsigned long RCID) const;


    void addFeature(const FeatureS57 & feat);
    void addSpatial(const SpatialS57 & spat);
    void addINode(const NodeS57 & feat);
    void addBNode(const NodeS57 & bNode);
    void addSoundg(const SoundgS57 & sndg);
    void addEdge(const EdgeS57 & edge);

    const std::multimap < LongNAMe, unsigned long > & getFOIDs()   const {return foid2rcid;} ///Key = FOID, value =Rec-ID of Feature Record("FE")
    unsigned long getRcid4Foid(LongNAMe foid) const
    {
        std::multimap < LongNAMe, unsigned long >::const_iterator it = foid2rcid.find(foid);
        if (it == foid2rcid.end()) return 0; 
        return it->second;
    }

protected:

    //**** methods ****
    void processBackwardPointers();
    void processBackwardPointers4newRecords();
    void processBackwardPointer4edge(const EdgeS57 *);
    void processBackwardPointer4feature(const FeatureS57 *);

    unsigned long getMaxRCID4FE() const;
    unsigned long getMaxRCID4VE() const;
    unsigned long getMaxRCID4VI() const;
    unsigned long getMaxRCID4VC() const;
    bool setRCIDifNeeded(SpatialS57 & spatial);

    void getSpatialsForFeature(const FeatureS57 & feat, std::vector<const SpatialS57 *> & spatials) const;
    void getSpatialsForFeature(const FeatureS57 & feat, std::vector<SpatialS57 *> & spatials);

    CheckResult checkResult;

    //**** Data (Cell Header is found in base-class) ****
    //** Features **
    std::map< unsigned long, FeatureS57 *> features;  ///Key = RecordId(RCID) of a Feature(RCNM is 100("FE"), anyway -> ignored)
    //** Vector Records are separately stored: Nodes (isolated and connecte), Soundings, and Edges **
    std::map< unsigned long, NodeS57 *> iNodes;       ///KEY: RecordId of an Isolated Node ("VI")
    std::map< unsigned long, BoundNodeS57 *> bNodes;  ///KEY: RecordId of an Bounding Node ("VC") 
    std::map< unsigned long, SoundgS57 *> soundings;  ///KEY: Rec-id of an Isolanted Node containing 3D-Cluster ("VI")
    std::map< unsigned long, EdgeS57 *> edges;        ///KEY: Rec-id of an Edge ("VE")

    //**** cached Data (needed 4 fast reocrd access) ****
    std::multimap < LongNAMe, unsigned long > foid2rcid; ///Key = FOID, value =Rec-ID of Feature Record("FE") - Used to find Feature To Feature Relationships quick

    //**** for updating: remeber new records ****
    std::vector<unsigned long> newFeatures; //newly added or modifyed Feature-Records
    std::vector<unsigned long> newEdges;    //newly added or modifyed Edge-Records
};
}
#endif