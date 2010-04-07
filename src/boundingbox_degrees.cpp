//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include "boundingbox_degrees.h"
#include <math.h>
#include <assert.h>

using namespace Enc;

#define DegEpsCompare 1E-6

DegBBox::DegBBox(const QString & bbString) : SLAT(0), WLON(0), NLAT(0), ELON(0)
{
    fromString(bbString);
}

double DegBBox::centerLon() const 
{
    if (stripe) return 0;
    else if (crossesDateLine())
    {
        double ret = (WLON + ELON + 360) /2.0;
        return (ret > 180 ? ret - 180.0 : ret);
    }
    return (ELON + WLON) / 2.0;
}

//******************************************************************************
/// Enlarge the bounding box by adding a single position
/*!
* This is done in a way that the smaller box will result
***************************************************************************** */
void DegBBox::add(double lat, double lon)
{
    if (!isValid())
    {
        SLAT = NLAT = lat;
        WLON = ELON = lon;
        assert(SLAT >= -90 && NLAT <= 90 && WLON >= -180 && ELON <= 180 && NLAT >= SLAT);
        return;
    }
    //** Trivial: Latitude **
    if (lat < SLAT) SLAT = lat;
    else if (lat > NLAT) NLAT = lat;

    //** Longitude: consider Dateline and minimal size **
    if (stripe) return;

    if (crossesDateLine())
    {
        //** take the shorter way (smaller box) **
        if (lon > ELON || lon < WLON) return;
        if (WLON - lon > lon - ELON) ELON = lon;
        else WLON = lon;
    }
    else 
    {
        if (lon < ELON && lon > WLON) return;
        //** take the shorter way -> maybe rect will cross dateline then **
        if (lon < WLON)
        {
            if (ELON - lon < 180) WLON = lon;
            else ELON = lon;
        }
        else if (lon > ELON)
        {
            if (lon - WLON < 180) ELON = lon;
            else WLON = lon;
        }
    }

    assert(SLAT >= -90 && NLAT <= 90 && WLON >= -180 && ELON <= 180 && NLAT >= SLAT);
}

void DegBBox::add(const DegBBox & otherBB)
{
    if (!isValid())
    {
        *this = otherBB;
        return;
    }
    //** Trivial: Latitude **
    if (otherBB.SLAT < SLAT) SLAT = otherBB.SLAT;
    else if (otherBB.NLAT > NLAT) NLAT = otherBB.NLAT;

    //** Longitude: consider Dateline and minimal size **
    if (stripe) return;
    if (otherBB.stripe)
    {
        stripe = true;
        return;
    }

    if (crossesDateLine())
    {
        //** 1) Trivial, too: both bboxes cross dateline: **
        if (otherBB.crossesDateLine())
        {
            if (otherBB.ELON >= WLON || otherBB.WLON >= ELON) stripe = true;
            else
            {
                if (otherBB.ELON > ELON) ELON = otherBB.ELON;
                if (otherBB.WLON < WLON) WLON = otherBB.WLON;
            }
        }
        //** 2) special: boxes create stripe: **
        else if (otherBB.WLON < ELON && otherBB.ELON > WLON)
        {
            stripe = true;
        }
        //** 3) normal case: take the shorter way (smaller box) **
        else
        {
            if (otherBB.ELON < ELON || otherBB.WLON < WLON) return; //completly include
            else if (otherBB.WLON < ELON) ELON = otherBB.ELON;      //overlapp
            else if (otherBB.ELON > WLON) WLON = otherBB.WLON;      //overlapp
            else                                                    //no contact
            {
                if ((otherBB.WLON - ELON) > (WLON - otherBB.ELON)) WLON = otherBB.WLON; 
                else ELON = otherBB.ELON;
            }
        }
    }
    else 
    {
        if (otherBB.crossesDateLine())
        {
            if (ELON <= otherBB.ELON || WLON >= otherBB.WLON)
            {
                ELON = otherBB.ELON;
                WLON = otherBB.WLON;
                return;
            }
            else if (ELON >= otherBB.WLON) ELON = otherBB.ELON;
            else if (WLON <= otherBB.ELON) WLON = otherBB.WLON;
            if ((WLON - otherBB.ELON) > (otherBB.WLON - ELON))
            {
                ELON = otherBB.ELON;   
            }
            else
            {
                WLON = otherBB.WLON;
            }
        }
        //** most likely: no BBox crosses Dateline **
        else
        {
            //** 1st - simple: if BBoxes overlapp **
            if (otherBB.WLON >= WLON && otherBB.ELON <= ELON) return;  //otherBB inside this BB
            else if (otherBB.WLON <= WLON && otherBB.ELON >= ELON)     //this BB inside otherBB 
            {
                WLON = otherBB.WLON;
                ELON = otherBB.ELON;
            }
            else if (otherBB.WLON < WLON && otherBB.ELON > WLON) WLON = otherBB.WLON; //real overlapp
            else if (otherBB.ELON > ELON && otherBB.WLON < ELON) ELON = otherBB.ELON; //real overlapp
            //** 2nd: boxes distant: calc smaller BBOX **
            else
            {
                //**1st: crossing datline makes smaller BBox: **
                if (fabs((ELON + WLON)/2 - (otherBB.ELON + otherBB.WLON)/2) > 180)
                {
                    if (WLON > otherBB.ELON) ELON = otherBB.ELON;
                    else                     WLON = otherBB.WLON;
                }
                //** 2nd: not crossing dateline makes smaller bbox: **
                else
                {
                    if (otherBB.WLON < WLON) WLON = otherBB.WLON;
                    else                     ELON = otherBB.ELON;
                }
            }
        }
    }
    assert(SLAT >= -90 && NLAT <= 90 && WLON >= -180 && ELON <= 180 && NLAT >= SLAT);
}


//******************************************************************************
/// Parse a Bounding box String 
/*!
* String must look like "S=1 W=2 N=3 E=4" or contain "STRIPE" instead of E and W
* (the oder of S,N,W,E does not matter)
***************************************************************************** */
void DegBBox::fromString(const QString & bbString) 
{
    //**** convert String like "S=%1 W=%2 N=%3 E=%4" or "STRIPE S=... N=..."****
    int indexS = bbString.indexOf("S=");
    int indexW = bbString.indexOf("W=");
    int indexN = bbString.indexOf("N=");
    int indexE = bbString.indexOf("E=");
    int indexStripe = bbString.indexOf("STRIPE");
    if (indexS >= 0 && indexN >= 0 && (indexStripe >= 0 || (indexE >= 0 && indexW >= 0)))
    {
        indexS += 2; indexW += 2;indexN += 2; indexE += 2;
        int endInd = bbString.indexOf(QChar(' '), indexS);
        SLAT = bbString.mid(indexS, (endInd>0 ? endInd - indexS: -1)).toDouble();
        endInd = bbString.indexOf(QChar(' '), indexN);
        NLAT = bbString.mid(indexN, (endInd>0 ? endInd - indexN : -1)).toDouble();
        if (indexStripe >= 0) stripe = true;
        else
        {
            endInd = bbString.indexOf(QChar(' '), indexW);
            WLON = bbString.mid(indexW, (endInd>0 ? endInd - indexW : -1)).toDouble();
            endInd = bbString.indexOf(QChar(' '), indexE);
            ELON = bbString.mid(indexE, (endInd>0 ? endInd - indexE : -1)).toDouble();
        }
    }
}

QString DegBBox::toString() const
{
    if (isValid())
    {
        if (stripe) return QString("STRIPE S=%1 N=%2").arg(SLAT).arg(NLAT);
        return QString("S=%1 W=%2 N=%3 E=%4").arg(SLAT).arg(WLON).arg(NLAT).arg(ELON);
    }
    return QString();
}

//******************************************************************************
/// Compare Bounding boxes, allowing small differences
/*!
* invalid Bounding boxes cannot be equal
****************************************************************************** */
bool DegBBox::operator== (const DegBBox & otherBB) const
{
    if (isValid() != otherBB.isValid() ) return false;
    if (!isValid()) return true; //both invalid -> treat as equal !!??
    if (fabs(SLAT - otherBB.SLAT) > DegEpsCompare || 
        fabs(NLAT - otherBB.NLAT) > DegEpsCompare || 
        fabs(WLON - otherBB.WLON) > DegEpsCompare || 
        fabs(ELON - otherBB.ELON) > DegEpsCompare) return false;
    return true;
}

//******************************************************************************
/// Check for really overlapping Bounding boxes (touching is not enought)
//****************************************************************************** */
bool DegBBox::overlapp(const DegBBox & otherBBox) const
{
   return  !(NLAT < otherBBox.SLAT ||
             SLAT > otherBBox.NLAT ||
             ELON < otherBBox.WLON ||
             WLON > otherBBox.ELON);
}