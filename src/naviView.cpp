#include "naviView.h"
#include "naviNaviWidgets.h"

#include <math.h>

#include "geo_projections.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QTransform>

using namespace Enc;

NaviView::NaviView(QWidget * parent) : QGraphicsView(parent), myAngleZ(0), myScale(1)
{
    //addNaviWidgets();
}

NaviView::NaviView(QGraphicsScene * scene, QWidget * parent): QGraphicsView(scene, parent), myAngleZ(0), myScale(1)
{

}

void NaviView::showContent(QRectF contentRect)
{
    setSceneRect(contentRect);
    update();
}

//*****************************************************************************
/// Draw Navigation Widgets onto the chart - optional
/*!
 * Signals are connected, too
 ****************************************************************************** */
void NaviView::addNaviWidgets()
{
    QHBoxLayout * mainLyt = new QHBoxLayout(viewport());
    QVBoxLayout * leftLyt = new QVBoxLayout();
    QVBoxLayout * rigthLyt = new QVBoxLayout();

    mainLyt->addLayout(leftLyt);
    mainLyt->addStretch();
    mainLyt->addLayout(rigthLyt);

    projectWgt = new ChartProjectionComboBox(this);
    scaleWgt = new ChartScaleWidget(this);
    headWgt = new ChartRotationWidget(this);
    posWgt = new ChartPositionWidget(this);
    xyWgt = new ChartEastNorthWidget(this);

    rigthLyt->addWidget(projectWgt);
    rigthLyt->addWidget(scaleWgt);
    rigthLyt->addWidget(headWgt);
    rigthLyt->addWidget(posWgt);
    rigthLyt->addWidget(xyWgt);

    rigthLyt->addStretch(1000);

    //**** inits ****
    initProjections();

    connect(projectWgt, SIGNAL(currentIndexChanged(int)), this, SIGNAL(projectionChanged(int)));
    connect(scaleWgt,  SIGNAL(zoonIn()), this, SLOT(zoomIn()));
    connect(scaleWgt, SIGNAL(zoomOut()), this, SLOT(zoomOut()));


    connect(this, SIGNAL(scaleChanged(double)), scaleWgt, SLOT(setScale(double)));
    connect(this, SIGNAL(headingChanged(double)), headWgt, SLOT(setHeading(double)));
}

void NaviView::initProjections()
{
   for (int prI=0; prI < ProjectionCount; ++prI) projectWgt->addItem(Projections[prI]);
}

/// Zoom in by 2.0
void NaviView::zoomIn()
{
    myScale = myScale *2.0;
    updateTransform();
    emit scaleChanged(myScale);
}

/// Zoom Out by 0.5
void NaviView::zoomOut()
{
    myScale = myScale *0.5;
    updateTransform();
    emit scaleChanged(myScale);
}

/// Set Scale if scale is really different
void NaviView::setScale(double newScale)
{
    if (fabs(myScale - newScale) < 0.5) return;
    myScale = newScale;
    updateTransform();
    emit scaleChanged(myScale);
}

//*****************************************************************************
/// Appy new chart heading to the view
/*!
 * Rem: compass heading has to be transformed into mathematical rotation
 ****************************************************************************** */
void NaviView::setChartHeading(double newHeading)
{
    double newAngleZ = 90.0 - newHeading; //mathematical angle
    if (fabs(newAngleZ - myAngleZ) < 0.1) return;

    myAngleZ = newAngleZ;
    updateTransform();
    emit headingChanged(90.0 - myAngleZ);
}

//*****************************************************************************
/// Appy the current transformation to the view
/*!
 * Signals about scale/rotation/translation are NOT emitted - caller must do that!
 ****************************************************************************** */
void NaviView::updateTransform()
{
    QTransform trans;
    trans.rotate(myAngleZ ,Qt::ZAxis);
    trans.scale(myScale, myScale);
    trans.translate(transform().dx(), transform().dy());
    setTransform(trans);
}
