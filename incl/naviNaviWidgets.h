#ifndef NAVINAVIWIDGETS_H
#define NAVINAVIWIDGETS_H

#include <QRectF>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QComboBox>

namespace Enc
{

//*****************************************************************************
/// Widget to display and change the Scale
/*!
  *
  *************************************************************************** */
class ChartScaleWidget : public QFrame
{
    Q_OBJECT

signals:
    void zoomIn();
    void zoomOut();

public:
    ChartScaleWidget(QWidget * parent =0);

public slots:
    void setScale(int scl);

protected:

    QPushButton * zoonInBtn, * zoomOutBtn;
    QLineEdit * scaleEdt;
};

//*****************************************************************************
/// Simple Widget to display a the current position
/*!
  *
  *************************************************************************** */
class ChartPositionWidget : public QFrame
{
    Q_OBJECT

public:
    ChartPositionWidget(QWidget * parent =0);

public slots:

    void setPosition(double lat, double lon);

protected:

    QLineEdit * latEdt, * lonEdt;
};

//*****************************************************************************
/// Simple Widget to display a the current Easting / Northing   (X,Y)position
/*!
  *
  *************************************************************************** */
class ChartEastNorthWidget : public QFrame
{
    Q_OBJECT

public:
    ChartEastNorthWidget(QWidget * parent =0);

public slots:

    void setEastNorth(double x, double y);

protected:

    QLineEdit * xEdt, * yEdt;
};

//*****************************************************************************
/// Simple Widget to display a the current Easting / Northing   (X,Y)position
/*!
  *
  *************************************************************************** */
class ChartRotationWidget : public QFrame
{
    Q_OBJECT

signals:
    void chartRotation(double);

public:
    ChartRotationWidget(QWidget * parent =0);

public slots:

    void setRotation(double rotation);

private slots:
    void onLeft() ; //const;
    void onRight() ; // const;

protected:

    QPushButton * negBtn, * posBtn;
    QLineEdit * rotEdt;
};


//*****************************************************************************
/// Select form a List of Projections
/*!
  *
  *************************************************************************** */
class ChartProjectionComboBox : public QComboBox
{
    Q_OBJECT

signals:

    void projectionChanged(int); //user changed projection, see Enc::Projections in geo_projections.h

public:
    ChartProjectionComboBox(QWidget * parent =0);

    int currentProjection() const {return currentIndex();}
};

}

#endif // NAVINAVIWIDGETS_H
