#ifndef NAVISCENEITEMS_H
#define NAVISCENEITEMS_H

#include <QtGui/QGraphicsPathItem>

namespace Enc
{

class GraphicsAreaFeatItem : public QGraphicsPathItem
{
public:
    enum {Type = UserType + 1};

    GraphicsAreaFeatItem(unsigned long rcid, QGraphicsItem * parent = 0);
    GraphicsAreaFeatItem(unsigned long rcid, const QPainterPath & path, const QPen & p, const QBrush & b, QGraphicsItem * parent = 0);
    unsigned long getRcId() const {return depareRcid;}
    virtual int type() const {return Type;}

private:
    unsigned long depareRcid;


};

class GraphicsDepareItem : public GraphicsAreaFeatItem
{
public:
    enum {Type = UserType + 2};
    GraphicsDepareItem(unsigned long rcid, const QPainterPath & path, const QPen & p, const QBrush & b, QGraphicsItem * parent = 0);
    virtual int type() const {return Type;}
};
}

#endif // NAVISCENEITEMS_H
