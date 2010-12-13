#include "naviSceneItems.h"

using namespace Enc;

GraphicsAreaFeatItem::GraphicsAreaFeatItem(unsigned long rcid, QGraphicsItem * parent) : QGraphicsPathItem(parent), depareRcid(rcid)
{}

GraphicsAreaFeatItem::GraphicsAreaFeatItem(unsigned long rcid, const QPainterPath & path, const QPen & p, const QBrush & b,  QGraphicsItem * parent)
                    : QGraphicsPathItem(path, parent), depareRcid(rcid)
{
    setPen(p);
    setBrush(b);
}

GraphicsDepareItem::GraphicsDepareItem(unsigned long rcid, const QPainterPath & path, const QPen & p, const QBrush & b, QGraphicsItem * parent)
                  : GraphicsAreaFeatItem(rcid, path, p, b, parent)
{}