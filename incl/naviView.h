#ifndef NAVIVIEW_H
#define NAVIVIEW_H

#include <QtGui/QGraphicsView>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QFrame>
#include <QtGui/QComboBox>

namespace Enc
{
class ChartProjectionComboBox;
class ChartScaleWidget;
class ChartRotationWidget;
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

signals:
    void projectionChanged(int);
    void scaleChanged(double);
    void headingChanged(double);


public:

    NaviView(QWidget * parent = 0);
    NaviView(QGraphicsScene * scene, QWidget * parent = 0);

    void initProjections();

public slots:

    void zoomIn();
    void zoomOut();
    void setScale(double);
    void setChartHeading(double heading);

    void showContent(QRectF);

protected:

    void updateTransform();
    void addNaviWidgets();

    ChartProjectionComboBox * projectWgt;
    ChartScaleWidget * scaleWgt;
    ChartRotationWidget * headWgt;
    ChartPositionWidget * posWgt;
    ChartEastNorthWidget * xyWgt;

    //** rember current Transformation **
    //kai: rest of tranformation paramters is taken directly from current view-transform
    double myAngleZ;    //angle (z-dirction) mathematical, not a compass-angle!
    double myScale;     //scale
};




}

#endif
