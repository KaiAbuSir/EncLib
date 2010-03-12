#include "cell_s57_iterators.h"

#include <math.h>

using namespace Enc;

const double FeatureVertexIterator::EPS = 0.0000001; //coordinate-difference less Eps means: equal!


FeatureVertexIterator::FeatureVertexIterator(const FeatureS57 * feature ,const CellS57_Base * cellS57) 
                     : bndryUserId(0), bndryId(0), finished(true), fsptInd(0), vertexInd(0), currentEdge(0), feat(feature), cell(cellS57), y_lat(0), x_lon(0)
{
    init();
}
FeatureVertexIterator::FeatureVertexIterator(unsigned long featRcid ,const CellS57_Base * cellS57) 
                     : bndryUserId(0), bndryId(0), finished(true), fsptInd(0), vertexInd(0), currentEdge(0), feat(0), cell(cellS57) ,y_lat(0), x_lon(0)
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
    //Orientation: 1,2,255 = Forward, Reverse, NULL 
    const BoundNodeS57 & bNode = cell->getBNode(feat->getFsptVec()[fsptInd].getORNT() == 1 ? currentEdge->getStartNodeRecId() : currentEdge->getEndNodeRecId());
    y_lat = bNode.getLat();
    x_lon = bNode.getLon();
}

bool FeatureVertexIterator::next()
{
    vertexInd += 2;
    double newYlat =0, newXlon=0;
    //**** normal case: use the next vertex ****
    if (vertexInd < currentEdge->getSG2Dvec().size())
    {
        if (feat->getFsptVec()[fsptInd].getORNT() == 1) //forward-edge
        {
            newYlat = currentEdge->getSG2Dvec()[vertexInd];
            newXlon = currentEdge->getSG2Dvec()[vertexInd +1];
        }
        else //reverse-edge
        { 
            newYlat = currentEdge->getSG2Dvec()[currentEdge->getSG2Dvec().size() - vertexInd -2];
            newXlon = currentEdge->getSG2Dvec()[currentEdge->getSG2Dvec().size() - vertexInd -1];
        }
    }
    //**** no vertex left: use the end-bounding-node ****
    else if (vertexInd == currentEdge->getSG2Dvec().size())
    {
        const BoundNodeS57 & bNode = cell->getBNode(feat->getFsptVec()[fsptInd].getORNT() == 1 ? currentEdge->getEndNodeRecId() : currentEdge->getStartNodeRecId());
        newYlat = bNode.getLat();
        newXlon = bNode.getLon(); 
    }
    //**** start new edge (if any) and use its start bounding node ****
    else
    {
        vertexInd = -2;
        ++fsptInd;
        if (fsptInd >= feat->getFsptVec().size())
        {
            bndryId = -1;
            return false;
        }
        else
        {
            if (bndryId == 0 && feat->getFsptVec()[fsptInd].getUSAG() != 1 && bndryId == 0 && feat->getFsptVec()[fsptInd].getUSAG() != 3)
            {
                ++bndryId;
                return false;
            }
            currentEdge = &(cell->getEdge(feat->getFsptVec()[fsptInd].getOtherRCID()));

            //Orientation: 1,2,255 = Forward, Reverse, NULL 
            const BoundNodeS57 & bNode = cell->getBNode(feat->getFsptVec()[fsptInd].getORNT() == 1 ? currentEdge->getStartNodeRecId() : currentEdge->getEndNodeRecId());
            newYlat = bNode.getLat();
            newXlon = bNode.getLon();
            //** rem: beginning of new INTERNAL boundary can only be detected by a coordiante gap **
            if ((fabs(y_lat - newYlat) > EPS) && (fabs(x_lon - newXlon) > EPS))
            {
                if (bndryId > 0 ) return false; //new INTERNAL boundary
                else
                {
                    throw QString("ERROR: In Feature: %1 boundary with gaps between: %1 %2").arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_FE, feat->getFRID().getRCID()))
                                                                                            .arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VE, feat->getFsptVec()[fsptInd -1].getOtherRCID()))
                                                                                            .arg(ISO8211::makeRecNameASCII(ISO8211::RCNM_VE, feat->getFsptVec()[fsptInd].getOtherRCID()));
                }
            }
        }
    }

    if (fabs(y_lat - newYlat) < EPS &&
        fabs(x_lon - newXlon) < EPS)
    {
        return next();
    }
    y_lat = newYlat;
    x_lon = newXlon;
    return true;
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
    while (bndryUserId != bndryId)
    {
        next();
        if (bndryId < 0) return false;
        
    }
    finished = false;
    return true;
}


void FeatureVertexIterator::rewind()
{
    fsptInd =0, vertexInd =0, currentEdge =0; 
    y_lat =0, x_lon =0;
    bndryUserId = 0;
    init();
}
