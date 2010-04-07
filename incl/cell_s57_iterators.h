#ifndef CELL_S57_ITERATORS
#define CELL_S57_ITERATORS

#include "cell_s57_base.h"

namespace Enc
{
class Projection;

//******************************************************************************
/// Iterator for ALL vertices of ALL edges of a Line/Area Feature
/*!
* Iterator skips redundant or nearly redundant vertices
* Iterator takes Bounding nodes as first/last vertex of edge, of course
* KAI: Man sollte gleich ein transformator mit übergeben, fuer:
*      1) projektion berechnen 2) transformation nach pixel koordinaten (integer) 
***************************************************************************** */
class FeatureVertexIterator
{
public:
    const static double EPS_DEG; // = 0.0000001; //for lat/lon(degress) coordinate-difference less Eps means: equal!
    const static double EPS_PROJ; // = 0.0001; //for north/east(meters)  coordinate-difference less EPS_PROJ means: equal!
    const double EPS;

    FeatureVertexIterator(const FeatureS57 * feature ,const CellS57_Base * cellS57, const Projection * prj =0, bool insertVertices = true);
    FeatureVertexIterator(unsigned long featRcid ,const CellS57_Base * cellS57, const Projection * prj =0, bool insertVertices = true);
    void operator++()
    {
        finished = !next();
    }
    bool valid() const
    {
        return !finished;
        //return currentEdge && (vertexInd <= (int)currentEdge->getSG2Dvec().size()) && (fsptInd < feat->getFsptVec().size());
    }
    void getValue(double & yOrlat, double & xOrlon) const 
    {
        yOrlat = (projection ? y_lat : lat); 
        xOrlon = (projection ? x_lon : lon);
    }
    double getYorLat() const {return (projection ? y_lat : lat);}
    double getXorLon() const {return (projection ? x_lon : lon);}
    bool nextBoundary();
    void rewind();

private:

    void init();
    bool next();
    bool interpolation(double & newLat, double & newLon);

    //**** current position of iterator ****
    double lat, lon; //current pos in degrees
    bool interpol; //true if current position was interpolated
    double y_lat, x_lon; //northing/easting: used if projection is applied

    //Boundary management: 0 means: outer boundary (always the first), 1 ... n: inner boundaries (if existant)
    int bndryUserId; //The boundary the user wants to iterate, currently
    int bndryId;      //current boundary
    bool finished;
    int fsptInd;
    int vertexInd;  //if less than 0 means: first bounding node
    const EdgeS57 * currentEdge;

    const FeatureS57 * feat;
    const CellS57_Base * cell;
    const Projection * projection;
    bool extraVert;
};

}
#endif