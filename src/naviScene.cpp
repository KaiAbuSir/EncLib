#include "naviScene.h"

#include "cell_s57_base.h"
#include "cell_s57_iterators.h"
#include "boundingbox_degrees.h"
#include "geo_projections.h"
#include "naviNaviWidgets.h"
#include "naviSceneItems.h"

#include<QtGui/QPen>
#include<QtGui/QBrush>
#include<QtGui/QPolygonF>
#include<QtGui/QPainterPath>
#include<QtGui/QAbstractGraphicsShapeItem>
#include<QtGui/QGraphicsLinearLayout>
#include<QtGui/QMessageBox>

using namespace Enc;

NaviScene::NaviScene(QObject * parent) : QGraphicsScene(parent), projectWgt(0), scaleWgt(0), posWgt(0), xyWgt(0), naviWgt(0),
                                         projectionId(0), projection(0), presenterS57(0)
{
    presenterS57 = new PresentationS52();
}
NaviScene::~NaviScene()
{
    clearAll();
    delete presenterS57;

}
 void NaviScene::setProjection(int prjctnId)
 {
     projectionId = prjctnId;  //waere besser, hier die projection zu erzeugen, oder?
 }

 //*****************************************************************************
 /// Add Navigation Widgets (if needed)
 /*!
  * Only used if Navi Widgets should be a part of the view, means: drawn on top to the chart
  ****************************************************************************** */
 void NaviScene::addNaviWidgets()
 {
     if (naviWgt) return; //dont add twice

     //**** create the widgets and connect them ****
     projectWgt = new ChartProjectionComboBox();
     scaleWgt = new ChartScaleWidget();
     posWgt = new ChartPositionWidget();
     xyWgt = new ChartEastNorthWidget();
     rotWgt = new ChartRotationWidget();

     //** Widget Signals are just forwarded with same-name Scene Signals **
     connect(projectWgt, SIGNAL(currentIndexChanged(int)), this, SIGNAL(projectionChanged(int)));
     connect(scaleWgt,  SIGNAL(zoonIn()), this, SIGNAL(zoomIn()));
     connect(scaleWgt, SIGNAL(zoomOut()), this, SIGNAL(zoomOut()));

     //** inits **
     for (int prI=0; prI < ProjectionCount; ++prI) projectWgt->addItem(Projections[prI]); // initProjections


     //**** create the Layout ****
     QGraphicsLinearLayout * mainLyt = new QGraphicsLinearLayout();
     QGraphicsLinearLayout * leftLyt = new QGraphicsLinearLayout(Qt::Vertical, mainLyt);
     mainLyt->addStretch();

     //** of course widgets need proxys **
     proxyMap[projectWgt] = addWidget(projectWgt);
     proxyMap[scaleWgt] = addWidget(scaleWgt);
     proxyMap[posWgt] = addWidget(posWgt);
     proxyMap[xyWgt] = addWidget(xyWgt);

     leftLyt->addItem(proxyMap[projectWgt]);
     leftLyt->addItem(proxyMap[scaleWgt]);
     leftLyt->addItem(proxyMap[posWgt]);
     leftLyt->addItem(proxyMap[xyWgt]);
     //unnötig: leftLyt->addStretch(1000);

     //**** finally, add all to the scene ****
     naviWgt = new QGraphicsWidget();
     naviWgt->setLayout(mainLyt);
     addItem(naviWgt);
     //kai: missing: diese widgets sollten von allen transformationen/scrolling ausgenommen werden
 }

//*****************************************************************************
/// Redraw all currently loaded cells
/*!
****************************************************************************** */
void NaviScene::onDrawCells()
{
    clear();  //remove old Graphic Objects

    //**** get the position of the cells to initalize the projection ****
    DegBBox viewBBox;
    for (uint cI = 0; cI < cells.size(); ++cI)
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
    
    int itemCnt = 0;
    const QGraphicsItem  * currItem = 0;

    QRectF viewBoundingRect;
    for (uint cI = 0; cI < cells.size(); ++cI)
    {
        CellS57_Base * newCell =cells[cI];
        if (!newCell) continue;
        emit progressMessage("Drawing cell: " + newCell->getDsid().getDSNM());

        try
        {
            const std::map< unsigned long, FeatureS57 *> & cellFeats = newCell->getFeatures();
            std::map< unsigned long, FeatureS57 *>::const_iterator fIt = cellFeats.begin();
            for (; fIt != cellFeats.end(); ++fIt)
            {
                    //if (cnt != CURRFEAT) {++cnt; continue;}  //debug only
                    currItem = convertFeature(fIt->first, fIt->second, newCell);
                    if (currItem)
                    {
                        viewBoundingRect = viewBoundingRect.unite(currItem->boundingRect());
                        //if (cnt == CURRFEAT) break;  //debug only
                        ++itemCnt;
                    }
            }
        }
        catch(const QString & msg)
        {
            QMessageBox::warning(NULL, "Cell Load Error", msg);
        }
    }

    invalidate();  //kai : braucht die Scene das????????????


    emit contentChanged(viewBoundingRect);
}

//*****************************************************************************
/// Convert a cell Feature into a SceneItem
/*!
* kai: not yet ready at all!
****************************************************************************** */
const QGraphicsItem * NaviScene::convertFeature(unsigned long RecId, const FeatureS57 * feat, CellS57_Base * newCell)
{
    QGraphicsItem * newItem = 0;
    if      (feat->getFRID().getPRIM() == 1) newItem = convertFeaturePoint(RecId, feat, newCell);
    else if (feat->getFRID().getPRIM() == 2) newItem = convertFeatureLine(RecId, feat, newCell);
    else if (feat->getFRID().getPRIM() == 3) newItem = convertFeatureArea(RecId, feat, newCell);
    if (newItem) newItem->setZValue(presenterS57->getPriority(feat));
    return newItem;
}

//*****************************************************************************
/// Convert a cell Feature with Point Geometry into a SceneItem
/*!
* rem: Point Symbols should have same size, independent of scale!
*
* kai: for now, just draw Simple Symbols,
* later: draw shapItems / pixmap items depending on Feature and Attributes
****************************************************************************** */
QGraphicsItem * NaviScene::convertFeaturePoint(unsigned long RecId, const FeatureS57 * feat, CellS57_Base * newCell)
{
    return 0;
}

//*****************************************************************************
/// Convert a cell Feature with Line Geometry into a SceneItem
/*!
* kai: for now, just draw polygons,
* later: draw shapItems/PolygoneItems depending on Feature and Attributes
****************************************************************************** */
QAbstractGraphicsShapeItem * NaviScene::convertFeatureLine(unsigned long RecId, const FeatureS57 * feat, CellS57_Base * newCell)
{
    QPen myPen = presenterS57->getPen(feat);  //colorfull debugging
   
    //** Lines have outer Boundary only, of course **
    QPolygonF edgeQP;
    FeatureVertexIterator vertexIt(feat , newCell, projection);
    for (;vertexIt.valid(); ++vertexIt)
    {
        edgeQP.push_back(QPointF(vertexIt.getXorLon(), -1* vertexIt.getYorLat()));
    }
    QGraphicsPolygonItem  * currItem = addPolygon(edgeQP, myPen);
    
    return currItem;
}

//*****************************************************************************
/// Convert a cell Feature with Area-Geometry into a SceneItem
/*!
* kai: for now, just draw simple Shape Items,
* later: draw shapItems depending on FeatureClass/Attributes
****************************************************************************** */
QAbstractGraphicsShapeItem * NaviScene::convertFeatureArea(unsigned long RecId, const FeatureS57 * feat, CellS57_Base * newCell)
{
    QPen myPen = presenterS57->getPen(feat);  //colorfull debugging
    QBrush myBrush = presenterS57->getBrush(feat);

    QPainterPath featPPath;

    //** outer Boundary **
    QPolygonF edgeQP;
    FeatureVertexIterator vertexIt(feat , newCell, projection);
    for (;vertexIt.valid(); ++vertexIt)
    {
        edgeQP.push_back(QPointF(vertexIt.getXorLon(), -1* vertexIt.getYorLat()));
    }
    featPPath.addPolygon(edgeQP) ; 
    //** get inner boundaries **
    int innerCnt =0;
    while (vertexIt.nextBoundary())
    {
        QPolygonF innerBoundaryQP;
//myPen.setStyle(Qt::PenStyle(innerCnt %4 +2));
        for (;vertexIt.valid(); ++vertexIt)
        {
            innerBoundaryQP.push_back(QPointF(vertexIt.getXorLon(), -1 * vertexIt.getYorLat()));
        }
        featPPath.addPolygon(innerBoundaryQP) ; 
        ++innerCnt;
    }

    QGraphicsPathItem * areaItem = 0;
    //**** Depare/Drgare are special, because their colors may change later ****
    if (ObjAttrDictionaryS57::codeDEPARE == feat->getFRID().getOBJL() ||
        ObjAttrDictionaryS57::codeDRGARE == feat->getFRID().getOBJL())
    {
        GraphicsDepareItem* depItem = new GraphicsDepareItem(RecId);
        depItem->setBrush(myBrush);
        depItem->setPen(myPen);
        addItem(depItem);
        areaItem = depItem;
    }
    else
    {
        areaItem = addPath(featPPath, myPen, myBrush);
        //if (!ObjAttrDictionaryS57::IsGroup1(feat->getFRID().getOBJL()))
    }
    return areaItem;
}

//*****************************************************************************
/// Clear all data and the Scene
/*!
****************************************************************************** */
void NaviScene::clearAll()
{
    for (uint cI = 0; cI < cells.size(); ++cI) delete cells[cI];
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
            emit progressMessage("Paring cell: " +*cIt);
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
    emit  progressMessage(QString("%1 cells loaded").arg(cellList.count()));
}
