#ifndef CELL_S57_ITERATORS
#define CELL_S57_ITERATORS

#include "cell_s57_base.h"

namespace Enc
{

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
    const static double EPS; // = 0.0000001; //coordinate-difference less Eps means: equal!

    FeatureVertexIterator(const FeatureS57 * feature ,const CellS57_Base * cellS57);
    FeatureVertexIterator(unsigned long featRcid ,const CellS57_Base * cellS57);
    void operator++()
    {
        finished = !next();
    }
    bool valid() const
    {
        return !finished;
        //return currentEdge && (vertexInd <= (int)currentEdge->getSG2Dvec().size()) && (fsptInd < feat->getFsptVec().size());
    }
    void getValue(double & yOrlat, double & xOrlon) const {yOrlat = y_lat; xOrlon = x_lon;}
    double getYorLat() const {return y_lat;}
    double getXorLon() const {return x_lon;}
    bool nextBoundary();
    void rewind();

private:

    void init();
    bool next();

    //**** current position of iterator ****
    double y_lat, x_lon;

    //Boundary management: 0 means: outer boundary (always the first), 1 ... n: inner boundaries (if existant)
    int bndryUserId; //The boundary the user wants to iterate, currently
    int bndryId;      //current boundary
    bool finished;
    int fsptInd;
    int vertexInd;  //if less than 0 means: first bounding node
    const EdgeS57 * currentEdge;

    const FeatureS57 * feat;
    const CellS57_Base * cell;

};

}
#endif