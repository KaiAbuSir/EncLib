#ifndef PRESENTATION_S52_H
#define PRESENTATION_S52_H

#include <vector>
#include <map>

#include <QtCore/QString>
#include <QtGui/QPen>
#include <QtGui/QBrush>

namespace Enc
{
class FeatureS57;
class FieldAttr;

//*****************************************************************************
/// Presentation Library - Gets the shape,color ... of Features depending on Attributes
/*!
  * Far from beeing complete - its ab big job!!
  *************************************************************************** */
class PresentationS52
{
public:
    PresentationS52();
    void setDeepShallow(double dp,double shlw, double vShlw);
    double getDeep() const {return deep;} 
    double getShallow() const {return shallow;} 
    double getVeryShallow() const {return veryShallow;}
    QPen getPen(const FeatureS57 * feat) const;
    QBrush getBrush(const FeatureS57 * feat) const;
    double getPriority(const FeatureS57 * feat) const;

private:
    //** just for colorfull debugging: **
    mutable int cnt; //how often a pen has been requested until now
    std::vector<QColor> colorVecDebug;  //just used to make chart colorfull for easier debugging

    //** group 1 Objects have predefined values **
    QBrush interTidalBrush, depareVeryShallowBrush, depareShallowBrush, depareMediumBrush, depareDeepBrush; //also for DRGARE
    QBrush LNDAREbrush, FLODOCbrush, HULKESbrush, PONTONbrush, UNSAREbrush;

    double deep, shallow, veryShallow; //deep shallow waters have different colours (in meters)
};

}

#endif