#ifndef CELL_RECORDS_H
#define CELL_RECORDS_H

#include <QtCore/QString>
#include <QtCore/QVariant>

#include <memory.h>
#include <vector>
#include <map>
#include <algorithm>

#include "boundingbox_degrees.h"
#include "cell_record_fields.h"

namespace Enc
{
class CellHeaderReader;
class CellParser8211;

#define Digits4EditionNo 4
#define Digits4UpdateNo 3


//******************************************************************************
/// Base class for all iso8211 Records
/*!
* Holds only the ISO8211 record-id, which is quite unimportant, just for information
***************************************************************************** */
class Record8211
{
public:
    enum RecType {Record, Feature, Spatial, Node, BNode, Sounding, Edge, FeatureUPDT, NodeUPDT, SoundingUPDT, EdgeUPDT};

    Record8211(unsigned long iso8211recid = 0) : recid8211(iso8211recid) {}
    virtual RecType getType() const {return Record;}
    virtual QString getNameASCII() const =0;
    virtual unsigned long long getNameBIN() const=0;
    virtual bool isOrphan() const =0; 

    unsigned long getISO8211recid() const {return recid8211;}
    void  setISO8211recid(unsigned long rcid8211)  {recid8211 = rcid8211;}

private:
    unsigned long recid8211; //ISO8211 record-id: NOT the S-57-RCID !!
};

//*****************************************************************************
/// Struct containig the data (Fields) of a complete Feature Record
/*!
* 
***************************************************************************** */
class FeatureS57 : public Record8211
{
public:
    FeatureS57(unsigned long iso8211recId =0) : Record8211(iso8211recId) {}
    virtual RecType getType() const {return Feature;}
    virtual QString getNameASCII() const {return ISO8211::makeRecNameASCII(FRID.getRCNM(), FRID.getRCID());}
    virtual unsigned long long getNameBIN() const {return ISO8211::makeRecName(FRID.getRCNM(), FRID.getRCID());}

    const FieldFRID & getFRID() const {return FRID;}
          FieldFRID & getFRID()       {return FRID;}

    const FieldFOID & getFOID() const {return FOID;}
          FieldFOID & getFOID()       {return FOID;}
          LongNAMe    getLongNAMe()   {return FOID.getLongNAMe();}

    const std::map<unsigned short, FieldAttr> & getAttribs() const {return attribs;}
          std::map<unsigned short, FieldAttr> & getAttribs()       {return attribs;}

    const std::vector < FieldFFPT > & getFfptVec() const {return FFPTvec;}
          std::vector < FieldFFPT > & getFfptVec()       {return FFPTvec;}

    const std::vector < FieldFSPT > & getFsptVec() const {return FSPTvec;}  
          std::vector < FieldFSPT > & getFsptVec()       {return FSPTvec;}

    virtual bool isOrphan() const {return FFPTvec.empty() && FSPTvec.empty() && ffptBackVec.empty();}

    const std::vector< unsigned long > & getFfptBackVec() const {return ffptBackVec;}
    /// Add Feat-Feat-Relations backward pointer, taking care not to insert twice!
    void addFfptBackPointer(unsigned long rcid)
    {
        if (std::find(ffptBackVec.begin(), ffptBackVec.end(), rcid) == ffptBackVec.end())
            ffptBackVec.push_back(rcid); //update reverse pointer
    }
    /// Remove Feat-Feat-Relations backward pointer 
    void remFfptBackPointer(unsigned long rcid)
    {
        std::vector <unsigned long>::iterator it = std::find(ffptBackVec.begin(), ffptBackVec.end(), rcid);
        if (it != ffptBackVec.end()) ffptBackVec.erase(it); 
    }

protected:
    FieldFRID FRID;
    FieldFOID FOID;
    std::map<unsigned short, FieldAttr> attribs;  //Holds normal AND national Attributes
                                                  //Attributes MUST NOT repeat -> we use a map

    std::vector < FieldFFPT > FFPTvec; //** Feature to Feature Relations: **
    std::vector < FieldFSPT > FSPTvec; //** Feature to Spatial pointer **
    std::vector <unsigned long> ffptBackVec; //** backward pointer of FFPTvec of other feats **
};

//******************************************************************************
/// Struct containig the data (Felds) of a complete UPDATE Feature Record
/*!
* Update Records hava all the base Fields, and 2 extra fields: FFPC and FSPC
***************************************************************************** */
class FeatureS57_Updt : public FeatureS57
{
public:
    FeatureS57_Updt(unsigned long iso8211recId =0) : FeatureS57(iso8211recId) {}
    virtual RecType getType() const {return FeatureUPDT;}

    const FieldFFPC & getFFPC() const {return FFPC;}
          FieldFFPC & getFFPC()       {return FFPC;}
    const FieldFSPC & getFSPC() const {return FSPC;}
          FieldFSPC & getFSPC()       {return FSPC;}

protected:
    FieldFFPC FFPC;
    FieldFSPC FSPC;
};

//******************************************************************************
/// Struct containig the data (subfields) of a complete Spatial Record
/*!
* 
***************************************************************************** */
class SpatialS57 : public Record8211
{
public:
#ifdef _DEBUG
    unsigned int recLenWrittn; //recordLen written when storing file - only to debug iso8211writer
#endif

    SpatialS57(unsigned long iso8211recId =0) : Record8211(iso8211recId) {} 
    SpatialS57(const FieldVRID & myVrid, unsigned long iso8211recId =0) : Record8211(iso8211recId), VRID(myVrid) {}
    virtual RecType getType() const {return Spatial;}
    virtual QString getNameASCII() const {return ISO8211::makeRecNameASCII(VRID.getRCNM(), VRID.getRCID());}
    virtual unsigned long long getNameBIN() const {return ISO8211::makeRecName(VRID.getRCNM(), VRID.getRCID());}

    const FieldVRID & getVRID() const {return VRID;}
          FieldVRID & getVRID()       {return VRID;}
    const std::map < unsigned short, QString > & getAttributes() const {return attibutes;} //key = ATTL, Val = Attribute Value ATVL
          std::map < unsigned short, QString > & getAttributes()       {return attibutes;} //key = ATTL, Val = Attribute Value ATVL
    const std::vector <unsigned long> & getFeatures() const {return features;}   //features connected to the spatial (backward pointer)

    void setVRID(const FieldVRID & val) {VRID = val;}
    void addFeature(unsigned long featRcid) /// update reverse pointer savely
    {
        if (std::find(features.begin(), features.end(), featRcid) == features.end())
            features.push_back(featRcid); 
    }
    void remFeature(unsigned long featRcid)
    {
        std::vector <unsigned long>::iterator it = std::find(features.begin(), features.end(), featRcid);
        if (it != features.end()) features.erase(it);  //testen ?? oder kann man auch einen ungueltigen iterator uebergeben
    }
protected:
    FieldVRID VRID;
    std::map < unsigned short, QString > attibutes; //key = ATTL, Val = Attribute Value ATVL
    std::vector <unsigned long> features;   //features connected to the spatial (backward pointer)
};

//******************************************************************************
/// Struct containig the data (subfields) of a complete Node Record
/*!
* Might be Isolated Node, or Bounding node of an Edge
***************************************************************************** */
class NodeS57 : public SpatialS57
{
public:
    NodeS57(unsigned long iso8211recId =0) : SpatialS57(iso8211recId), lat(0), lon(0) {}
    NodeS57(const FieldVRID & myVrid, unsigned long iso8211recId =0) : SpatialS57(myVrid, iso8211recId), lat(0), lon(0) {}
    NodeS57(double _lat, double _lon) : lat(_lat), lon(_lon) {}
    virtual RecType getType() const {return Node;}

    double getLat() const {return lat;}
    double getLon() const {return lon;}
    double & getLat()  {return lat;}
    double & getLon()  {return lon;}
    void setLatLon(double newLat, double newLon) {lat = newLat, lon = newLon;}

    virtual bool isOrphan() const {return features.empty();}

protected:
    double lat, lon; //lat/lon (Y,X) coordiates, but already transformed by to double with  COMF
};

//******************************************************************************
/// Struct containig the data (subfields) of a complete Connected Node Record
/*!
* Holds Backward pointer to the edges which are bound by the node
***************************************************************************** */
class BoundNodeS57 : public NodeS57
{
public:
    BoundNodeS57(unsigned long iso8211recId =0) : NodeS57(iso8211recId) {}
    BoundNodeS57(const FieldVRID & myVrid, unsigned long iso8211recId =0) : NodeS57(myVrid, iso8211recId) {}
    BoundNodeS57(const NodeS57 & node) : NodeS57(node) 
    {
        VRID.setRCNM(ISO8211::RCNM_VC);
    }
    virtual RecType getType() const {return BNode;}

    const std::vector <unsigned long> getEdges() const {return edges;} //Record ID of Edges bound by this node (backward pointer)

    /// add Edge (backward pointer) take care not to add edge twice
    void addEdge(unsigned long edgeRcid) 
    {
        if (std::find(edges.begin(), edges.end(), edgeRcid) == edges.end()) edges.push_back(edgeRcid);
    }
    /// remove edge (backward pointer) return true on success 
    bool remEdge(unsigned long edgeRcid)
    {
        std::vector <unsigned long>::const_iterator it = std::find(edges.begin(), edges.end(), edgeRcid);
        if (it != edges.end()){
            edges.erase(it);
            return true;
        }
        return false;
    }
    bool isOrphan() const
    {
        return edges.empty() && features.empty();
    }

protected:
    std::vector <unsigned long> edges; //Record ID of Edges bound by this node (backward pointer)
};

//******************************************************************************
/// Struct containig the data (subfields) of a 3D-Field Spatial Record (Soundings)
/*!
* Might be anything, but: in S-57 ENCs, only Soundings are coded as 3D Fields
***************************************************************************** */
class SoundgS57 : public SpatialS57
{
public:
    SoundgS57(unsigned long iso8211recId =0) : SpatialS57(iso8211recId) {}
    SoundgS57(const FieldVRID & myVrid, unsigned long iso8211recId =0) : SpatialS57(myVrid, iso8211recId) {}
    virtual RecType getType() const {return Sounding;}

    const std::vector < double > & getSG3Dvec() const {return SG3Dvec;} ///lat/lon/depth (Y,X,Z) coordiates, but already transformed by to double with  COMF and SOMF
          std::vector < double > & getSG3Dvec()       {return SG3Dvec;} ///lat/lon/depth (Y,X,Z) coordiates, but already transformed by to double with  COMF and SOMF

    virtual bool isOrphan() const {return features.empty();}

    const DegBBox & getBoundingBox();

protected:
    std::vector < double > SG3Dvec; ///lat/lon/depth (Y,X,Z) coordiates, but already transformed by to double with  COMF and SOMF
    DegBBox bBox;
};

//******************************************************************************
/// Struct containig the data (subfields) of a complete Edge Record
/*!
* 
***************************************************************************** */
class EdgeS57 : public SpatialS57
{
public:
    EdgeS57(unsigned long iso8211recId =0) : SpatialS57(iso8211recId), startNodeRecId(0), endNodeRecId(0) {}
    EdgeS57(const FieldVRID & myVrid, unsigned long iso8211recId =0) : SpatialS57(myVrid, iso8211recId) {}
    virtual RecType getType() const {return Edge;}
  
    unsigned long getStartNodeRecId() const {return startNodeRecId;}
    unsigned long getEndNodeRecId() const {return endNodeRecId;} 

    const std::vector < double > & getSG2Dvec()const {return SG2Dvec;} /// Get lat/lon (Y,X) coordiates, but already transformed by to double with  COMF
         std::vector < double > & getSG2Dvec()       {return SG2Dvec;} /// Get lat/lon (Y,X) coordiates, but already transformed by to double with  COMF

    void setStartNodeRecId(unsigned long val)  { startNodeRecId = val;}
    void setEndNodeRecId(unsigned long val)  { endNodeRecId = val;} 

    virtual bool isOrphan() const {return features.empty();}
    const DegBBox & getBoundingBox() const {return bBox;}
    void setBoundingBox(const DegBBox & box) {bBox = box;} 

protected:
    //** Instead of using FieldVRPT, just remeber the Start/End Nodes
    unsigned long startNodeRecId;
    unsigned long endNodeRecId; 

    std::vector < double > SG2Dvec; //lat/lon (Y,X) coordiates, but already transformed by to double with  COMF
    DegBBox bBox;
};

//******************************************************************************
/// Struct containig the update informatino needed for a spatial Update Record
/*!
***************************************************************************** */
class SpatialS57_Updt 
{
public:
    const FieldVRPC & getVrpc() const {return VRPC;}
          FieldVRPC & getVrpc()       {return VRPC;}
    const FieldSGCC & getSgcc() const {return SGCC;}
          FieldSGCC & getSgcc()       {return SGCC;}
protected:
    FieldVRPC VRPC;
    FieldSGCC SGCC;
};

//******************************************************************************
/// Node UPDATE Record , might be Isolated or Connected Node
/*!
***************************************************************************** */
class NodeS57_Updt : public NodeS57, public SpatialS57_Updt
{
public:
    NodeS57_Updt(unsigned long iso8211recId =0) : NodeS57(iso8211recId) {}
    NodeS57_Updt(const FieldVRID & myVrid, unsigned long iso8211recId =0) : NodeS57(myVrid, iso8211recId) {}
    virtual RecType getType() const {return NodeUPDT;}
};

//******************************************************************************
/// Sounding UPDATE Record
/*!
***************************************************************************** */
class SoundgS57_Updt : public SoundgS57, public SpatialS57_Updt
{
public:
    SoundgS57_Updt(unsigned long iso8211recId =0) : SoundgS57(iso8211recId) {}
    SoundgS57_Updt(const FieldVRID & myVrid, unsigned long iso8211recId =0) : SoundgS57(myVrid, iso8211recId) {}
    virtual RecType getType() const {return SoundingUPDT;}
};

//******************************************************************************
/// Edge UPDATE Record
/*!
***************************************************************************** */
class EdgeS57_Updt : public EdgeS57, public SpatialS57_Updt
{
public:
    EdgeS57_Updt(unsigned long iso8211recId =0) : EdgeS57(iso8211recId) {}
    EdgeS57_Updt(const FieldVRID & myVrid, unsigned long iso8211recId =0) : EdgeS57(myVrid, iso8211recId) {}
    virtual RecType getType() const {return EdgeUPDT;}
};

}
#endif


