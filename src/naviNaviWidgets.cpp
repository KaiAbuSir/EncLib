#include "naviNaviWidgets.h"


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

using namespace Enc;

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
/// Simple Widget to display a the current Easting / Northing   (X,Y)position
/*!
  *
  *************************************************************************** */
ChartRotationWidget::ChartRotationWidget(QWidget * parent) : QFrame(parent)
{

    negBtn = new QPushButton("Left", this);
    posBtn = new QPushButton("Right", this);;
    rotEdt = new QLineEdit(" 180 � ", this);
    QHBoxLayout * lyt = new QHBoxLayout(this);
    lyt->addWidget(negBtn);
    lyt->addWidget(rotEdt);
    lyt->addWidget(posBtn);

    setFixedSize(sizeHint());

    connect(negBtn, SIGNAL(clicked()), this, SLOT(onLeft()));
    connect(posBtn, SIGNAL(clicked()), this, SLOT(onRight()));
}

//*****************************************************************************
/// Set rotation angle - Caller must take care that angle is [-180,180] deg.
/*! decimals are not displayed!
  *************************************************************************** */
void ChartRotationWidget::setRotation(double rot)
{
    rotEdt->setText(QString("%1 �").arg((int)rot));
}

//*****************************************************************************
/// Request a left-rotation of the chart
/*!
  * The new rotation angle is not set yet, but must be set by caller after rotation finished
  *************************************************************************** */
void ChartRotationWidget::onLeft() //const
{
    double newRotation = rotEdt->text().toDouble();
    newRotation -= 10.0;
    emit chartRotation(newRotation);
}

//*****************************************************************************
///  Request a right-rotation of the chart
/*!
  * The new rotation angle is not set yet, but must be set by caller after rotation finished
  *************************************************************************** */
void ChartRotationWidget::onRight() //const
{
    double newRotation = rotEdt->text().toDouble();
    newRotation += 10.0;
    emit chartRotation(newRotation);
}

//*****************************************************************************
/// Select form a List of Projections
/*!
  *
  *************************************************************************** */
ChartProjectionComboBox::ChartProjectionComboBox(QWidget * parent): QComboBox(parent)
{

}
