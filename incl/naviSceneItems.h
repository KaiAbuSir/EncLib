#ifndef NAVISCENEITEMS_H
#define NAVISCENEITEMS_H

#include <QtGui/QGraphicsPathItem>

namespace Enc
{

class GraphicsDepareItem : public QGraphicsPathItem
{
public:
    GraphicsDepareItem(unsigned long rcid, QGraphicsItem * parent = 0);
    unsigned long getRcId() const {return depareRcid;}

private:
    unsigned long depareRcid;


};
}

#endif // NAVISCENEITEMS_H
