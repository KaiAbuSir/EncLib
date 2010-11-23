#ifndef NAVISCENE_H
#define NAVISCENE_H

#include "dictionaryS52.h"

#include <vector>

#include <QGraphicsScene>
#include <QAbstractGraphicsShapeItem>
#include <QPen>

namespace Enc
{
class CellS57_Base;
class Projection;
class FeatureS57;


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
    void contentChanged();

public:
    NaviScene(QObject * parent = 0 );
    virtual ~NaviScene();

public slots:
    void loadCharts(QStringList filenames);
    void setProjection(int prjctnId);
    void onDrawCells();
    void clearAll();

protected:

    const QAbstractGraphicsShapeItem * convertFeature(unsigned long, const FeatureS57 * feat, CellS57_Base *);

    int projectionId;
    Projection * projection;

    std::vector<Enc::CellS57_Base *> cells;

    PresentationS52 * presenterS57;
};



}
#endif // NAVISCENE_H
