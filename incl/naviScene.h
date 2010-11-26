#ifndef NAVISCENE_H
#define NAVISCENE_H

#include "dictionaryS52.h"

#include <vector>

#include <QWidget>
#include <QGraphicsScene>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsProxyWidget>
#include <QPen>
#include <QRectF>

namespace Enc
{
class CellS57_Base;
class Projection;
class FeatureS57;

class ChartProjectionComboBox;
class ChartScaleWidget;
class ChartPositionWidget;
class ChartEastNorthWidget;

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
    void rotateRight(double);
    void projectionChanged(int);

public:
    NaviScene(QObject * parent = 0 );
    virtual ~NaviScene();

public slots:
    void loadCharts(QStringList filenames);
    void setProjection(int prjctnId);
    void onDrawCells();
    void clearAll();

protected:

    //**** GUI methods ****
    void addNaviWidgets();

    //**** Cell-drawing methods ****
    const QAbstractGraphicsShapeItem * convertFeature(unsigned long, const FeatureS57 * feat, CellS57_Base *);

    //**** navigation widgets on the scene (added by using proxy-widgets) ****
    ChartProjectionComboBox * projectWgt;
    ChartScaleWidget * scaleWgt;
    ChartPositionWidget * posWgt;
    ChartEastNorthWidget * xyWgt;

    QMap <QWidget*, QGraphicsProxyWidget*> proxyMap;

    //**** data needed to draw the cell-contents ****
    int projectionId;
    Projection * projection;

    std::vector<Enc::CellS57_Base *> cells;

    PresentationS52 * presenterS57;
};



}
#endif // NAVISCENE_H
