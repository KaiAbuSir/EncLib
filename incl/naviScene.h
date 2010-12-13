#ifndef NAVISCENE_H
#define NAVISCENE_H

#include "dictionaryS57.h"
#include "presentationS52.h"

#include <vector>

#include <QtGui/QWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QAbstractGraphicsShapeItem>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QPen>
#include <QtCore/QRectF>

namespace Enc
{
class CellS57_Base;
class Projection;
class FeatureS57;

class ChartProjectionComboBox;
class ChartScaleWidget;
class ChartPositionWidget;
class ChartEastNorthWidget;
class ChartRotationWidget;

//*****************************************************************************
/// Scene holding the projected items of a number of ENCs
/*!
  *
  *************************************************************************** */
class NaviScene : public QGraphicsScene
{
Q_OBJECT

signals:

    void progressMessage(const QString &);
    void contentChanged(QRectF);

    //** forwarding signals of navi-widgets **
    void zoomIn();
    void zoomOut();
    void chartHeading(double);
    void projectionChanged(int);

public:
    NaviScene(QObject * parent = 0 );
    virtual ~NaviScene();
    void addNaviWidgets();

public slots:
    void loadCharts(QStringList filenames);
    void setProjection(int prjctnId);
    void onDrawCells();
    void clearAll();

protected:

    //**** GUI methods ****


    //**** Cell-drawing methods ****
    const QGraphicsItem * convertFeature(unsigned long, const FeatureS57 * feat, CellS57_Base *);
    QGraphicsItem *              convertFeaturePoint(unsigned long, const FeatureS57 * feat, CellS57_Base *);
    QAbstractGraphicsShapeItem * convertFeatureLine(unsigned long, const FeatureS57 * feat, CellS57_Base *);
    QAbstractGraphicsShapeItem * convertFeatureArea(unsigned long, const FeatureS57 * feat, CellS57_Base *);


    //**** navigation widgets on the scene (added by using proxy-widgets) ****
    ChartProjectionComboBox * projectWgt;
    ChartScaleWidget * scaleWgt;
    ChartPositionWidget * posWgt;
    ChartEastNorthWidget * xyWgt;
    ChartRotationWidget * rotWgt;
    QGraphicsWidget * naviWgt;  //holds ALL navi, widgets, but may be NULL

    QMap <QWidget*, QGraphicsProxyWidget*> proxyMap;

    //**** data needed to draw the cell-contents ****
    int projectionId;
    Projection * projection;

    std::vector<Enc::CellS57_Base *> cells;

    PresentationS52 * presenterS57;
};



}
#endif // NAVISCENE_H
