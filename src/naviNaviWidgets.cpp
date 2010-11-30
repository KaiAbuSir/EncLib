#include "naviNaviWidgets.h"

#include <QtCore/QRegExp>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

using namespace Enc;

AllNaviWidgets::AllNaviWidgets(QWidget * wgtParent)
{
    projectWgt = new ChartProjectionComboBox(wgtParent);
    scaleWgt = new ChartScaleWidget(wgtParent);
    posWgt = new ChartPositionWidget(wgtParent);
    xyWgt = new ChartEastNorthWidget(wgtParent);
    rotWgt = new ChartRotationWidget(wgtParent);
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
    scaleEdt = new QLineEdit(" -------- ", this);
    zoonInBtn = new QPushButton("+", this);
    zoomOutBtn = new QPushButton("-", this);
    lyt->addWidget(zoomOutBtn , 0);
    lyt->addWidget(scaleEdt , 1);
    lyt->addWidget(zoonInBtn , 0);

    setFixedHeight(sizeHint().height());

    connect(zoonInBtn, SIGNAL(clicked()), this, SIGNAL(zoomIn()));
    connect(zoomOutBtn, SIGNAL(clicked()), this, SIGNAL(zoomOut()));
}

void ChartScaleWidget::setScale(int scl)
{
    scaleEdt->setText(QString::number(scl));
}

void ChartScaleWidget::setScale(double scl)
{
    if (scl < 0) scaleEdt->setText(QString::number(scl));
    scaleEdt->setText(QString::number((int)scl));
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

    setFixedHeight(sizeHint().height());
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
    xEdt = new QLineEdit(" ------- ", this);
    yEdt = new QLineEdit(" ------- ", this);
    lyt->addWidget(new QLabel(tr(" Easting="),this), 0);
    lyt->addWidget(xEdt, 1);
    lyt->addWidget(new QLabel(tr(" Northing="),this), 0);
    lyt->addWidget(yEdt , 1);

    setFixedHeight(sizeHint().height());
}

void ChartEastNorthWidget::setEastNorth(double x, double y)
{
    xEdt->setText(QString::number(x));
    yEdt->setText(QString::number(y));
}

//*****************************************************************************
/// Simple Widget to display a the current Easting / Northing   (X,Y)position
/*!
  *
  *************************************************************************** */
ChartRotationWidget::ChartRotationWidget(QWidget * parent) : QFrame(parent)
{

    negBtn = new QPushButton("Left", this);
    posBtn = new QPushButton("Right", this);;
    rotEdt = new QLineEdit("    0 ° ", this);
    QHBoxLayout * lyt = new QHBoxLayout(this);
    lyt->addWidget(new QLabel("Heading: ", this));
    lyt->addWidget(negBtn);
    lyt->addWidget(rotEdt);
    lyt->addWidget(posBtn);

    setFixedHeight(sizeHint().height());

    connect(negBtn, SIGNAL(clicked()), this, SLOT(onLeft()));
    connect(posBtn, SIGNAL(clicked()), this, SLOT(onRight()));
}

//*****************************************************************************
/// Set heading angle - Caller must take care that 0-angle is north.
/*! decimals are not displayed!
  *************************************************************************** */
void ChartRotationWidget::setHeading(double heading)
{
    rotEdt->setText(QString(" %1 °").arg((int)heading));
}

//*****************************************************************************
/// Request a left-rotation of the chart
/*!
  * The new rotation angle is not set yet, but must be set by caller after rotation finished
  *************************************************************************** */
void ChartRotationWidget::onLeft() //const
{
    bool numOk;
    double newRotation = rotEdt->text().replace(QRegExp("°"), " ").simplified().toDouble(&numOk);
    newRotation -= 10.0;
    emit chartHeading(newRotation);
}

//*****************************************************************************
///  Request a right-rotation of the chart
/*!
  * The new rotation angle is not set yet, but must be set by caller after rotation finished
  *************************************************************************** */
void ChartRotationWidget::onRight() //const
{
    bool numOk;
    double newRotation = rotEdt->text().replace(QRegExp("°"), " ").simplified().toDouble(&numOk);
    newRotation += 10.0;
    emit chartHeading(newRotation);
}

//*****************************************************************************
/// Select form a List of Projections
/*!
  *
  *************************************************************************** */
ChartProjectionComboBox::ChartProjectionComboBox(QWidget * parent): QComboBox(parent)
{

}
