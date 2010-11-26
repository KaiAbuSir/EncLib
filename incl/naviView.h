#ifndef NAVIVIEW_H
#define NAVIVIEW_H

#include <QGraphicsView>
#include <QRectF>
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

signals:
    void projectionChanged(int);

public:

    NaviView(QWidget * parent = 0);
    NaviView(QGraphicsScene * scene, QWidget * parent = 0);

    void initProjections();

public slots:

    void zoomIn();
    void zoomOut();
    void rotateRight(double);

    void showContent(QRectF);

protected:

    void addNaviWidgets();

    ChartProjectionComboBox * projectWgt;
    ChartScaleWidget * scaleWgt;
    ChartPositionWidget * posWgt;
    ChartEastNorthWidget * xyWgt;

};




}

#endif
