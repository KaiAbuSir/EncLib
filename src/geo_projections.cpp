#include "geo_projections.h"
#include "geo_spheroids.h"

#include "GeographicLib/LocalCartesian.hpp"
#include "GeographicLib/AzimuthalEquidistant.hpp"
#include "GeographicLib/CassiniSoldner.hpp"
#include "GeographicLib/Geocentric.hpp"
#include "GeographicLib/LambertConformalConic.hpp"
#include "GeographicLib/PolarStereographic.hpp"
#include "GeographicLib/TransverseMercator.hpp"
#include "GeographicLib/TransverseMercatorExact.hpp"
#include "GeographicLib/UTMUPS.hpp"



char * Enc::Projections [] ={ "No Projection",
                                "Plattkarte",
                                "Azimuthal Equidistant",
                                "Cassini Soldner",
                                "Lambert Conformal Conic",
                                "Polar Stereographic",
                                "Mercator",
                                "Transverse Mercator",
                                "Transverse Mercator Exact",
                                "UTM"};
int Enc::ProjectionCount = 10;

 //Enc::projectionList << "No Projection"    << "Plattkarte";
using namespace Enc;

Projection * Projection::getProjection(int projectionId, double centerLat, double centerLon)
{
    Projection * prjctn = 0;
    if      (projectionId == Platt)         prjctn = new Plattkarte();
    else if (projectionId == AziEquiDist)   prjctn = new AzimuthalEquidistant(centerLat, centerLon);
    else if (projectionId == CassiSold)     prjctn = new CassiniSoldner(centerLat, centerLon);
    else if (projectionId == LambConfCon)   prjctn = new LambertConformalConic(centerLat, centerLat, centerLon);
    else if (projectionId == PolarStereo)   prjctn = new PolarStereographic();
    else if (projectionId == NormalMercator)prjctn = new Mercator();
    else if (projectionId == TMercator)     prjctn = new TransverseMercator(centerLon);
    else if (projectionId == TMercatorExact)prjctn = new TransverseMercatorExact(centerLon);
    else if (projectionId == UTMercator)    prjctn = new UTM(centerLat, centerLon);
    return prjctn;
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
Plattkarte::Plattkarte(double lat0, double lon0)
{
    localCartesian = new GeographicLib::LocalCartesian(lat0, lon0);
}
Plattkarte::~Plattkarte()
{
    delete localCartesian;
}
void Plattkarte::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double dummyH =0, dummyZ =0;
    localCartesian->Forward(lat, lon, dummyH,x, y, dummyZ); 
}

void Plattkarte::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    double dummyH =0, dummyZ =0;
    localCartesian->Reverse(x, y, dummyZ, lat, lon, dummyH);
}   

//*****************************************************************************
/// 
/*!
****************************************************************************** */
Mercator::Mercator()
{

}

void Mercator::latLon2xy(double lat, double lon, double & x, double & y) const
{

}
void Mercator::xy2LatLon(double x, double y, double & lat, double & lon) const
{

}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
AzimuthalEquidistant::AzimuthalEquidistant(double _lat0, double _lon0) : lat0(_lat0), lon0(_lon0)
{
    azimuthalEquidistant = new GeographicLib::AzimuthalEquidistant;
}
AzimuthalEquidistant::~AzimuthalEquidistant()
{
    delete azimuthalEquidistant;
}
void AzimuthalEquidistant::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double azi, rk;
    azimuthalEquidistant->Forward(lat0, lon0, lat, lon, x, y, azi, rk); 
}
void AzimuthalEquidistant::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    double azi, rk;
    azimuthalEquidistant->Reverse(lat0, lon0, x, y, lat, lon, azi, rk);
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
CassiniSoldner::CassiniSoldner(double lat0, double lon0)
{
    cassiniSoldner = new GeographicLib::CassiniSoldner(lat0, lon0);
}

CassiniSoldner::~CassiniSoldner()
{
    delete cassiniSoldner;
}
void CassiniSoldner::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double azi, rk;
    cassiniSoldner->Forward(lat, lon, x, y, azi, rk);
}
void CassiniSoldner::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    double azi, rk;
    cassiniSoldner->Reverse(x, y, lat, lon, azi, rk);
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
LambertConformalConic::LambertConformalConic(double stdlat1, double stdlat2, double _lon0, double k1) : lon0(_lon0)
{
    lambertConformalConic = new GeographicLib::LambertConformalConic(WGS84::MajorAxis, WGS84::InversFlat, stdlat1, stdlat2, k1);
}

LambertConformalConic::~LambertConformalConic()
{
    delete lambertConformalConic;
}

void LambertConformalConic::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double gamma =0, k =0;
    lambertConformalConic->Forward(lon0, lat, lon, x, y, gamma, k);
}

void LambertConformalConic::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    double gamma =0, k =0;
    lambertConformalConic->Reverse(lon0, x, y, lat, lon, gamma, k);
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
PolarStereographic::PolarStereographic()
{
    polarStereo = new GeographicLib::PolarStereographic(WGS84::MajorAxis, WGS84::InversFlat, 1);

}

PolarStereographic::~PolarStereographic()
{
    delete polarStereo;
}

void PolarStereographic::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double  gamma, k;
    polarStereo->Forward(true, lat, lon, x, y, gamma, k);
}

void PolarStereographic::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    double  gamma, k;
    polarStereo->Reverse(true, x, y, lat, lon, gamma, k);
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
TransverseMercator::TransverseMercator(double _lon0, double k0) : lon0(_lon0)
{
    transversMercator = new GeographicLib::TransverseMercator(WGS84::MajorAxis, WGS84::InversFlat, k0);
}
TransverseMercator::~TransverseMercator()
{
    delete transversMercator;
}

void TransverseMercator::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double gamma=0, k=0;
    transversMercator->Forward(lon0, lat, lon, x, y, gamma, k);
}

void TransverseMercator::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    double gamma=0, k=0;
    transversMercator->Reverse(lon0, x, y, lat, lon, gamma, k);
}
//*****************************************************************************
/// 
/*!
****************************************************************************** */
TransverseMercatorExact::TransverseMercatorExact(double _lon0, double k0) : lon0(_lon0)
{
    transverseMercatorExact = new GeographicLib::TransverseMercatorExact(WGS84::MajorAxis, WGS84::InversFlat, k0);
}

TransverseMercatorExact::~TransverseMercatorExact()
{
    delete transverseMercatorExact;
}

void TransverseMercatorExact::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double gamma=0, k=0;
    transverseMercatorExact->Forward(lon0, lat, lon, x,  y, gamma, k);
}

void TransverseMercatorExact::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    double gamma=0, k=0;
    transverseMercatorExact->Reverse(lon0, x,  y, lat, lon, gamma, k);
}

//*****************************************************************************
/// 
/*!
****************************************************************************** */
UTM::UTM(double centerLat, double centerLon) 
{
    zone = GeographicLib::UTMUPS::StandardZone(centerLat,centerLon);
}

UTM::~UTM()
{}

void UTM::latLon2xy(double lat, double lon, double & x, double & y) const
{
    double gamma, k;
    bool northp;
    int currentZone;
    GeographicLib::UTMUPS::Forward(lat, lon, currentZone, northp, x, y, gamma, k);
    if (currentZone != zone)
    {
        //correct x (easting) 
    }
}

void UTM::xy2LatLon(double x, double y, double & lat, double & lon) const
{
    //** check if x > zone limits **
    int currentZone = zone;
    if (fabs(x) > 180000)
    {
        //correct zone ??
    }

    double gamma, k;
    GeographicLib::UTMUPS::Reverse(currentZone, true, x, y, lat, lon, gamma, k);
}

