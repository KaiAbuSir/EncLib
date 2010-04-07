//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "cell_s57_iterators.h"
#include "geo_projections.h"

#include <math.h>

using namespace Enc;

const double FeatureVertexIterator::EPS_DEG = 0.0000002; //coordinate-difference less Eps means: equal!
const double FeatureVertexIterator::EPS_PROJ = 0.00005; //difference only 0.1mm on earth means: equal!


FeatureVertexIterator::FeatureVertexIterator(const FeatureS57 * feature ,const CellS57_Base * cellS57, const Projection * prj, bool insertVertices) 
                     : EPS(prj ? EPS_PROJ : EPS_DEG), bndryUserId(0), bndryId(0), finished(true), fsptInd(0), vertexInd(0), currentEdge(0), feat(feature), cell(cellS57), 
                       lat(0), lon(0), interpol(false), y_lat(0), x_lon(0), projection(prj), extraVert(insertVertices)
{
    init();
}
FeatureVertexIterator::FeatureVertexIterator(unsigned long featRcid ,const CellS57_Base * cellS57, const Projection * prj, bool insertVertices) 
                     : EPS(prj ? EPS_PROJ : EPS_DEG), bndryUserId(0), bndryId(0), finished(true), fsptInd(0), vertexInd(0), currentEdge(0), feat(0), cell(cellS57),
                       lat(0), lon(0), interpol(false), y_lat(0), x_lon(0), projection(prj), extraVert(insertVertices)
{
    if (!cellS57) throw QString("ERROR: Feature Iterator without cell created!");
    feat = &cellS57->getFeature(featRcid);
}

void FeatureVertexIterator::init()
{
    if (!feat || !cell) throw QString("ERROR: Cannot init FeatureVertexIterator");
    if (feat->getFsptVec().empty()) return;
    fsptInd = 0;
    if (feat->getFsptVec()[fsptInd].getOtherRCNM() == ISO8211::RCNM_VE)
    {
        currentEdge = &(cell->getEdge(feat->getFsptVec()[fsptInd].getOtherRCID()));
    }
    else throw QString("ERROR Creating Vertex Iterator, but no edges referenced by Feature: " + makeRecNameASCII(feat->getFRID().getRCNM(), feat->getFRID().getRCID()));

    finished = false;

    vertexInd = -2;
    //REM: Orientation: 1,2,255 = Forward, Reverse, NULL 
    const BoundNodeS57 & bNode = cell->getBNode(feat->getFsptVec()[fsptInd].getORNT() == 1 ? currentEdge->getStartNodeRecId() : currentEdge->getEndNodeRecId());
    lat = bNode.getLat(); 
    lon = bNode.getLon();
    if (projection)
    {
        projection->latLon2xy(lat, lon, x_lon, y_lat);
    }
}

//*****************************************************************************
/// go to the next Vertex, skip redundant vertices
/*!
* returns false, if 1) End Reached, or: 
*                   2) End of current boundry reached (external or internal)
* REM: if false is returned, the iterator points to 1st vertex of NEXT boundary, if there is any,
* or to last valid of last edge, if end is reached
****************************************************************************** */   
bool FeatureVertexIterator::next()
{
    bool sameEdge = true;

    //**** if current vertex is interpolated, no need to get next real vertex ****
    if (!interpol) vertexInd += 2;
    
    double newLat =0, newLon=0;
    //**** normal case: use the next vertex ****
    if (vertexInd < currentEdge->getSG2Dvec().size())
    {
        if (feat->getFsptVec()[fsptInd].getORNT() == 1) //forward-edge
        {
            newLat = currentEdge->getSG2Dvec()[vertexInd];
            newLon = currentEdge->getSG2Dvec()[vertexInd +1];
        }
        else //reverse-edge
        { 
            newLat = currentEdge->getSG2Dvec()[currentEdge->getSG2Dvec().size() - vertexInd -2];
            newLon = currentEdge->getSG2Dvec()[currentEdge->getSG2Dvec().size() - vertexInd -1];
        }
    }
    //**** no vertex left: use the end-bounding-node ****
    else if (vertexInd == currentEdge->getSG2Dvec().size())
    {
        const BoundNodeS57 & bNode = cell->getBNode(feat->getFsptVec()[fsptInd].getORNT() == 1 ? currentEdge->getEndNodeRecId() : currentEdge->getStartNodeRecId());
        newLat = bNode.getLat();
        newLon = bNode.getLon(); 
    }
    //**** start new edge (if any) and use its start bounding node ****
    else
    {
        vertexInd = -2;
        ++fsptInd;
        if (fsptInd >= feat->getFsptVec().size())
        {
            bndryId = -1;
            sameEdge = false; //NO MORE VERTICES AT ALL
        }
        else
        {
            currentEdge = &(cell->getEdge(feat->getFsptVec()[fsptInd].getOtherRCID()));
            //Orientation: 1,2,255 = Forward, Reverse, NULL 
            const BoundNodeS57 & bNode = cell->getBNode(feat->getFsptVec()[fsptInd].getORNT() == 1 ? currentEdge->getStartNodeRecId() : currentEdge->getEndNodeRecId());
            newLat = bNode.getLat();
            newLon = bNode.getLon();
            
            //** beginning of FIRST internal boundary can be detected by USAG value **
            if (bndryId == 0 && feat->getFsptVec()[fsptInd].getUSAG() != 1 && bndryId == 0 && feat->getFsptVec()[fsptInd].getUSAG() != 3)
            {
                ++bndryId; //first INTERNAL boundary reached
                sameEdge = false;
            }
            //** rem: beginning of new INTERNAL boundary can only be detected by a coordiante gap **
            else if ((fabs(lat - newLat) > EPS_DEG) && (fabs(lon - newLon) > EPS_DEG))
            {
                if (bndryId > 0 ) 
                {
                    ++bndryId;
                    sameEdge = false; //new INTERNAL boundary
                }
                else
                {
                    throw QString("ERROR: In Feature: %1 boundary with gaps between: %1 %2").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_FE, feat->getFRID().getRCID()))
                                                                                            .arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VE, feat->getFsptVec()[fsptInd -1].getOtherRCID()))
                                                                                            .arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VE, feat->getFsptVec()[fsptInd].getOtherRCID()));
                }
            }
        }
    }

    if (sameEdge) interpol = interpolation(newLat, newLon);
    else interpol = false;

    //** skip redundant vertices, but: start/end vertices of edge are never redundant! **
    if (sameEdge && !interpol && fabs(lat - newLat) < EPS_DEG && fabs(lon - newLon) < EPS_DEG)
    {
        return next();
    }
    lat = newLat;
    lon = newLon;

    if (projection)
    {
        double newX =0, newY =0;
        projection->latLon2xy(newLat, newLon, newX, newY);
        y_lat = newY;
        x_lon = newX;
    }
    
    return sameEdge;
}

//*****************************************************************************
/// go to the next Boundary
/*!
* Normaly, the next boundary is already reached 
* (if user iterated through previous bounday completely)
****************************************************************************** */   
bool FeatureVertexIterator::nextBoundary()
{
    if (bndryId < 0) return false;
    ++bndryUserId;
    interpol = false;

    while (bndryUserId != bndryId)
    {
        next();
        if (bndryId < 0) return false;
        
    }
    finished = false;

    //REM: Orientation: 1,2,255 = Forward, Reverse, NULL 
    const BoundNodeS57 & bNode = cell->getBNode(feat->getFsptVec()[fsptInd].getORNT() == 1 ? currentEdge->getStartNodeRecId() : currentEdge->getEndNodeRecId());

    lat = bNode.getLat(); 
    lon = bNode.getLon();
    if (projection)
    {
        projection->latLon2xy(lat, lon, x_lon, y_lat);
    }
    return true;
}


void FeatureVertexIterator::rewind()
{
    fsptInd =0, vertexInd =0, currentEdge =0; 
    lat = 0, lon = 0, y_lat =0, x_lon =0;
    interpol = false;
    bndryUserId = 0;
    init();
}


//*****************************************************************************
/// insert extra vertices if needed
/*!
* Alter newLat / newLon if distance to old position is more than 2 degrees.
* hint: most likley cases: horizontal and vertical lines! 
* returns true if vertices where inserted
****************************************************************************** */   
bool FeatureVertexIterator::interpolation(double & newLat, double & newLon)
{
    if (!extraVert) return false;
    //** if less than 2 deg. diff or near the pole: do nothing:
    if ((fabs(newLat - lat) <= 2.0 && fabs(newLon - lon) <= 2.0) || (fabs(newLat) > 85 && fabs(lat) > 85)) return false;

    //** most simple: vertical line **
    if (fabs(newLon -lon) < EPS_DEG)
    {
        (newLat > lat ?  newLat = lat + 2.0 : newLat = lat - 2.0);
    }
    else
    {
        bool forward = newLon > lon;
        if (fabs(newLon - lon) > 180) //crossing date line? 
        {
            forward = !forward;
        }
        //**** simple: horizontal line (but my cross dateline!)****
        if (fabs(newLat - lat) < EPS_DEG)
        {
            if (forward) newLon = lon + 2.0;
            else         newLon = lon - 2.0;
        }
        //**** seldom case: long oblique line: calculate without time consuming (trigonometric ...) methods: ****
        else
        {
            double tanAlpha = (newLat - lat) / (newLon - lon);
            if (fabs(tanAlpha) < 0.999) 
            {
                if (forward) newLon = lon + 1.4;
                else         newLon = lon - 1.4;
                newLat = lat + tanAlpha * 1.4;
            }
            else if (fabs(tanAlpha) > 1.001) 
            {
                if (newLat > lat) newLat = lat + 1.4;
                else              newLat = lat - 1.4;
                newLon = lon + 1.4 / tanAlpha;
            }
            //not unusual: 45 deg line
            else  
            {
                if (forward) newLon = lon + 1.4;
                else         newLon = lon - 1.4;
                if (newLat > lat) newLat = lat + 1.4;
                else              newLat = lat - 1.4;
            }
        }
        //correct dateline crossing
        if (newLon > 180) newLon -= 360; 
        if (newLon < -180) newLon += 360; 
    }
    return true;
}
