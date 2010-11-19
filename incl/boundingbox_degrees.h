#ifndef BOUNDING_BOX_DEGREES_H
#define BOUNDING_BOX_DEGREES_H

#include <QtCore/QString>

namespace Enc
{

//******************************************************************************
/// Bounding Box in Decimal Degrees (south-west and north-east corners)
/*!
* BBox works correctly even when bigger than 180 degress, even until 360 degrees,
***************************************************************************** */
struct DegBBox
{
    DegBBox() : SLAT(0), WLON(0), NLAT(0), ELON(0), stripe(false)
    {}
    DegBBox(double _SLAT, double _WLON, double _NLAT, double _ELON) : SLAT(_SLAT), WLON(_WLON), NLAT(_NLAT), ELON(_ELON), stripe(false)
    {}
    DegBBox(const QString & bbString);
    void clear() {SLAT = 0; WLON = 0; NLAT = 0; ELON = 0;}
    bool isValid() const {return (SLAT != 0 || WLON != 0 || NLAT != 0 || ELON != 0) && (NLAT >= SLAT);} 
    bool crossesDateLine() const {return WLON > ELON;}
    QString toString() const;
    void fromString(const QString & bbString);

    void add(double lat, double lon);
    void add(const DegBBox & otherBB);
    double centerLat() const{ return (NLAT + SLAT)/2.0;}
    double centerLon() const;

    double SLAT, WLON;  //South-West (LowerLeft) Corner;
    double NLAT, ELON;  //North-East (UpperRight) Corner;
    bool stripe;        //if true: W-E-long have no meaning: bbox is a stripe covering whole globe from SLAT to NLAT
  
    bool operator<(const DegBBox & otherBB) const {return (simpleSize() < otherBB.simpleSize());}  /// get the boundingbox with the smaller area
    bool operator== (const DegBBox & otherBB) const;
    bool operator!= (const DegBBox & otherBB) const {return !(*this == otherBB);}
    double simpleSize() const {return (NLAT - SLAT) * deltaLon();} /// Calculate the area without using a projection -> quick
    bool overlapp(const DegBBox & otherBBox) const;

private:
    double deltaLon() const {return (crossesDateLine() ? (ELON + 360) - WLON : ELON -WLON);}
};

}

#endif
