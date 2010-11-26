#include "naviView.h"
#include "naviNaviWidgets.h"

#include "geo_projections.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

using namespace Enc;

NaviView::NaviView(QWidget * parent) : QGraphicsView(parent)
{
    //addNaviWidgets();
}

NaviView::NaviView(QGraphicsScene * scene, QWidget * parent): QGraphicsView(scene, parent)
{

}

void NaviView::showContent(QRectF contentRect)
{
    setSceneRect(contentRect);
    update();
}

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
    posWgt = new ChartPositionWidget(this);
    xyWgt = new ChartEastNorthWidget(this);

    rigthLyt->addWidget(projectWgt);
    rigthLyt->addWidget(scaleWgt);
    rigthLyt->addWidget(posWgt);
    rigthLyt->addWidget(xyWgt);

    rigthLyt->addStretch(1000);

    //**** inits ****
    initProjections();

    connect(projectWgt, SIGNAL(currentIndexChanged(int)), this, SIGNAL(projectionChanged(int)));
    connect(scaleWgt,  SIGNAL(zoonIn()), this, SLOT(zoomIn()));
    connect(scaleWgt, SIGNAL(zoomOut()), this, SLOT(zoomOut()));
}

void NaviView::initProjections()
{
   for (int prI=0; prI < ProjectionCount; ++prI) projectWgt->addItem(Projections[prI]);
}

void NaviView::zoomIn()
{
    scale(2.0, 2.0);
}

void NaviView::zoomOut()
{
    scale(0.5, 0.5);
}

void NaviView::rotateRight(double rotation)
{

}
