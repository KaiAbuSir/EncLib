#include "naviView.h"

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

//*****************************************************************************
/// Widget to display and change the Scale
/*!
  *
  *************************************************************************** */
ChartScaleWidget::ChartScaleWidget(QWidget * parent) :  QFrame(parent)
{
    QHBoxLayout * lyt = new QHBoxLayout(this);

    //**** scale ****
    lyt->addWidget(new QLabel(tr(" Scale: "),this), 0);
    scaleEdt = new QLineEdit(this);
    zoonInBtn = new QPushButton("+", this);
    zoomOutBtn = new QPushButton("-", this);
    lyt->addWidget(zoomOutBtn , 0);
    lyt->addWidget(scaleEdt , 1);
    lyt->addWidget(zoonInBtn , 0);

    connect(zoonInBtn, SIGNAL(clicked()), this, SIGNAL(zoomIn()));
    connect(zoomOutBtn, SIGNAL(clicked()), this, SIGNAL(zoomOut()));
}

void ChartScaleWidget::setScale(int scl)
{
    scaleEdt->setText(QString::number(scl));
}

//*****************************************************************************
/// Widget to display the position (Lat / Lon)
/*!
  *
  *************************************************************************** */
ChartPositionWidget::ChartPositionWidget(QWidget * parent) :  QFrame(parent)
{
    QHBoxLayout * lyt = new QHBoxLayout(this);

    //**** Position lat/lon ****
    lyt->addWidget(new QLabel(tr(" Position: "),this), 0);
    latEdt = new QLineEdit(this);
    lonEdt = new QLineEdit(this);
    lyt->addWidget(latEdt, 1);
    lyt->addWidget(lonEdt, 1);
}

void ChartPositionWidget::setPosition(double lat, double lon)
{
  latEdt->setText(QString::number(lat));
  lonEdt->setText(QString::number(lon));
}

//*****************************************************************************
/// Widget to display Easting / Northing (X and Y)
/*!
  *
  *************************************************************************** */
ChartEastNorthWidget::ChartEastNorthWidget(QWidget * parent ): QFrame(parent)
{
    QHBoxLayout * lyt = new QHBoxLayout(this);

    //**** northing/easting (x,y) ****
    xEdt = new QLineEdit(this);
    yEdt = new QLineEdit(this);
    lyt->addWidget(new QLabel(tr(" Easting="),this), 0);
    lyt->addWidget(xEdt, 1);
    lyt->addWidget(new QLabel(tr(" Northing="),this), 0);
    lyt->addWidget(yEdt , 1);
}

void ChartEastNorthWidget::setEastNorth(double x, double y)
{
    xEdt->setText(QString::number(x));
    yEdt->setText(QString::number(y));
}


//*****************************************************************************
/// Select form a List of Projections
/*!
  *
  *************************************************************************** */
ChartProjectionComboBox::ChartProjectionComboBox(QWidget * parent): QComboBox(parent)
{

}

/*
//**** Projection ****
projectionCBx = new QComboBox(this);


*/
