#ifndef NAVIVIEW_H
#define NAVIVIEW_H

#include <QGraphicsView>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QComboBox>

namespace Enc
{
class ChartProjectionComboBox;
class ChartScaleWidget;
class ChartPositionWidget;
class ChartEastNorthWidget;
//*****************************************************************************
/// Widget to display a Chart and appropriate Navi-Buttons
/*!
  * Kai : i am trying to put widgets on the view, not on the scene - weiss noch nicht ob und wie das geht
  *************************************************************************** */
class NaviView : public QGraphicsView
{
    Q_OBJECT

public:

    NaviView(QWidget * parent = 0);
    NaviView(QGraphicsScene * scene, QWidget * parent = 0);

public slots:

    void zoomIn();
    void zoomOut();

protected:

    void addNaviWidgets();

    ChartProjectionComboBox * projectWgt;
    ChartScaleWidget * scaleWgt;
    ChartPositionWidget * posWgt;
    ChartEastNorthWidget * xyWgt;

};

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
/// Select form a List of Projections
/*!
  *
  *************************************************************************** */
class ChartProjectionComboBox : public QComboBox
{
    Q_OBJECT

signals:

    void projectionChange(int); //user changed projection, see Enc::Projections in geo_projections.h

public:
    ChartProjectionComboBox(QWidget * parent =0);

    int currentProjection() const {return currentIndex();}
};


}

#endif
