#include "naviScene.h"

#include "cell_s57_base.h"
#include "cell_s57_iterators.h"
#include "boundingbox_degrees.h"
#include "geo_projections.h"

#include<QPen>
#include<QBrush>
#include<QPolygonF>
#include<QAbstractGraphicsShapeItem >
#include<QMessageBox>

using namespace Enc;

NaviScene::NaviScene(QObject * parent) : QGraphicsScene(parent), projectionId(0), projection(0)
{

}
NaviScene::~NaviScene()
{
    clearAll();

}
 void NaviScene::setProjection(int prjctnId)
 {
     projectionId = prjctnId;  //waere besser, hier die projection zu erzeugen, oder?
 }

//*****************************************************************************
/// Redraw all currently loaded cells
/*!
****************************************************************************** */
void NaviScene::onDrawCells()
{
    clearAll();  //remove old content

    //QBrush brushRed(QColor(255,0,0), Qt::Dense3Pattern);
    //QBrush brushGreen(QColor(0,255,0), Qt::Dense4Pattern);
    //const int CURRFEAT = 1; //just 4 debug

    //**** get the position of the cells to initalize the projection ****
    DegBBox viewBBox;
    for (int cI = 0; cI < cells.size(); ++cI)
    {
        viewBBox.add(cells[cI]->getBBox());
    }
    if (projection) delete projection;
    projection=0;
    projection = Projection::getProjection(projectionId, viewBBox.centerLat(), viewBBox.centerLon());
    double yMin =viewBBox.SLAT, yMax =viewBBox.NLAT, xMin =viewBBox.ELON, xMax =viewBBox.WLON;
    if (projection)
    {
        projection->latLon2xy(viewBBox.SLAT, viewBBox.WLON, xMin, yMin);
        projection->latLon2xy(viewBBox.NLAT, viewBBox.ELON, xMax, yMax);
    }
    bool red = true;
    const QAbstractGraphicsShapeItem  * currItem = 0;

    for (int cI = 0; cI < cells.size(); ++cI, red = !red)
    {
        try
        {
            CellS57_Base * newCell =cells[cI];

            int cnt = 0;
            const std::map< unsigned long, FeatureS57 *> & cellFeats = newCell->getFeatures();
            std::map< unsigned long, FeatureS57 *>::const_iterator fIt = cellFeats.begin();
            for (; fIt != cellFeats.end(); ++fIt)
            {
                if (fIt->second->getFRID().getPRIM() == 3 ) //&& fIt->second->getFRID().getGRUP() == 1 )
                {
                    //if (cnt != CURRFEAT) {++cnt; continue;}  //debug only
                    currItem = convertFeature(fIt->first, fIt->second, newCell);
                    //if (cnt == CURRFEAT) break;  //debug only
                    ++cnt;
                }
            }
        }
        catch(const QString & msg)
        {
            QMessageBox::warning(NULL, "Cell Load Error", msg);
        }
    }

    emit contentChanged();
}

//*****************************************************************************
/// Convert a cell Feature into a SceneItem
/*!
* kai: for now, just draw polygons,
* later: draw shapItems / pixmap items
****************************************************************************** */
const QAbstractGraphicsShapeItem * NaviScene::convertFeature(unsigned long, const FeatureS57 * feat, CellS57_Base * newCell)
{
    const QAbstractGraphicsShapeItem  * currItem;

    QPen myPen = dict.getPen(); //colorfull debugging

    QPolygonF edgeQP;
    FeatureVertexIterator vertexIt(feat , newCell, projection);
    for (;vertexIt.valid(); ++vertexIt)
    {
        edgeQP.push_back(QPointF(vertexIt.getXorLon(), -1* vertexIt.getYorLat()));
    }
    currItem = addPolygon(edgeQP, myPen) ; //, (red ? brushRed : brushGreen));
    //** get inner boundaries **
    int innerCnt =0;
    while (vertexIt.nextBoundary())
    {
        QPolygonF innerBoundaryQP;
myPen.setStyle(Qt::PenStyle(innerCnt %4 +2));
        for (;vertexIt.valid(); ++vertexIt)
        {
            innerBoundaryQP.push_back(QPointF(vertexIt.getXorLon(), -1 * vertexIt.getYorLat()));
        }
        currItem = addPolygon(innerBoundaryQP, myPen) ; //, (red ? brushRed : brushGreen));
        ++innerCnt;
    }
    return currItem;
}

//*****************************************************************************
/// Clear all data and the Scene
/*!
****************************************************************************** */
void NaviScene::clearAll()
{
    for (int cI = 0; cI < cells.size(); ++cI) delete cells[cI];
    cells.clear();
    clear();
}

//*****************************************************************************
/// Clear old data and load new Cells into the Scene
/*!
****************************************************************************** */
void NaviScene::loadCharts(QStringList cellList)
{
    clearAll();
    for (QStringList::iterator cIt = cellList.begin(); cIt != cellList.end(); ++cIt)
    {
        try
        {
            CellS57_Base * newCell = new CellS57_Base();
            newCell->parseISO8211(*cIt);
            cells.push_back(newCell);
        }
        catch(const QString & msg)
        {
            QMessageBox::warning(0, "Cell Load Error", msg);  //kai: should throw exception ?????????
        }
    }
    onDrawCells();
}
