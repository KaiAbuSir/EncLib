#ifndef GEO_PROJECTIONS_H
#define GEO_PROJECTIONS_H

#include <QtCore/QStringList>

namespace GeographicLib 
{
class LocalCartesian;
class AzimuthalEquidistant;
class CassiniSoldner;
class LambertConformalConic;
class TransverseMercator;
class TransverseMercatorExact;
class PolarStereographic;
}

namespace Enc
{
    extern char * Projections [];
    extern int ProjectionCount;
    //extern QStringList projectionList;
    

class Projection
{
public: 
    enum ProjectionId{NoProjec=0,Platt, AziEquiDist, CassiSold, LambConfCon, PolarStereo, NormalMercator, TMercator, TMercatorExact, UTMercator};
    static Projection * getProjection(int projectionId, double centerLat =0, double centerLon=0);
    virtual void latLon2xy(double lat, double lon, double & x, double & y) const = 0;
    virtual void xy2LatLon(double x, double y, double & lat, double & lon) const = 0;

};

class Plattkarte : public Projection
{
public:
    Plattkarte(double lat0 =0, double lon0 =0);
    ~Plattkarte();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:

    GeographicLib::LocalCartesian * localCartesian;
};

class AzimuthalEquidistant : public Projection
{
public:
    AzimuthalEquidistant(double _lat0 =0, double _lon0 =0);
    ~AzimuthalEquidistant();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:
    double lat0, lon0;
    GeographicLib::AzimuthalEquidistant * azimuthalEquidistant;
};

class CassiniSoldner : public Projection
{
public:
    CassiniSoldner(double lat0 =0, double lon0 =0);
    ~CassiniSoldner();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:
    GeographicLib::CassiniSoldner * cassiniSoldner;
};

class LambertConformalConic : public Projection
{
public:
    LambertConformalConic(double stdlat1 =0, double stdlat2 =0, double _lon0 =0, double k1 =1);
    ~LambertConformalConic();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:
    double lon0;
    GeographicLib::LambertConformalConic * lambertConformalConic;
};

class PolarStereographic : public Projection
{
public:
    PolarStereographic();
    ~PolarStereographic();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:

    GeographicLib::PolarStereographic * polarStereo;
};

class Mercator : public Projection
{
public:
    Mercator();
    ~Mercator();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:

};

class TransverseMercator : public Projection
{
public:
    TransverseMercator(double _lon0 =0, double k0 =1);
    ~TransverseMercator();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:

    double lon0;
    GeographicLib::TransverseMercator * transversMercator;
};
 
class TransverseMercatorExact : public Projection
{
public:
    TransverseMercatorExact(double _lon0 =0, double k0 =1);
    ~TransverseMercatorExact();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:
    double lon0;
    GeographicLib::TransverseMercatorExact * transverseMercatorExact;
};

class UTM  : public Projection
{
public:
    UTM(double centerLat =0, double centerLon =0);
    ~UTM();
    void latLon2xy(double lat, double lon, double & x, double & y) const;
    void xy2LatLon(double x, double y, double & lat, double & lon) const;

protected:

    int zone;
};

}



#endif

