#include "presentationS52.h"
#include "dictionaryS57.h"

#include <QtCore/QFile>

#include <map>


#include "cell_records.h"

using namespace Enc;

//*****************************************************************************
/// Dictionary - just a dummy, will become a real dict later
/*!
  *
  *************************************************************************** */
PresentationS52::PresentationS52() : cnt(0), deep(20), shallow(3), veryShallow(1)
{
    //** Default Brushes for Group1 Objects **
    interTidalBrush = QBrush(QColor(139,226,179, 200));       //kais hellgruen, nur zum testen
    depareVeryShallowBrush = QBrush(QColor(196,247,236, 200));//kais ganz hell blau, nur zum testen
    depareShallowBrush = QBrush(QColor(187,241,251, 200));    //kais hell blau, nur zum testen
    depareMediumBrush = QBrush(QColor(27,137,217, 200));      //kais mittelblau, nur zum testen
    depareDeepBrush = QBrush(QColor(18,47,146, 200));         //kais dunkelblau, nur zum testen

    LNDAREbrush = QBrush(QColor(231,164,24, 200)); //kais braun, nur zum testen
    FLODOCbrush = QBrush(QColor(231,164,24, 200)); //kais braun, nur zum testen
    HULKESbrush = QBrush(QColor(231,164,24, 200)); //kais braun, nur zum testen
    PONTONbrush = QBrush(QColor(231,164,24, 200)); //kais braun, nur zum testen
    UNSAREbrush = QBrush(QColor(205,254,254, 200)); //kais hellblau, nur zum testen



    //**** just used 4 debug ****
    colorVecDebug.push_back((QColor(0,0,0)));
    colorVecDebug.push_back((QColor(125,0,0)));
    colorVecDebug.push_back((QColor(250,0,0)));
    colorVecDebug.push_back((QColor(0,125,0)));
    colorVecDebug.push_back((QColor(0,250,0)));
    colorVecDebug.push_back((QColor(0,0,125)));
    colorVecDebug.push_back((QColor(0,0,250)));
    colorVecDebug.push_back((QColor(20,50,100)));
    colorVecDebug.push_back((QColor(100,50,20)));
    colorVecDebug.push_back((QColor(50,100,20)));
    colorVecDebug.push_back((QColor(20,100,50)));
    colorVecDebug.push_back((QColor(50,20,100)));
    colorVecDebug.push_back((QColor(100,20,50)));
}

void PresentationS52::setDeepShallow(double dp, double shlw, double vShlw)
{
    deep = dp; shallow = shlw; veryShallow = vShlw;

    if (veryShallow > shallow) veryShallow = shallow;
    if (deep < shallow) deep = shallow;
}

//*****************************************************************************
/// Return the pen to draw a line-geometry item
/*!
  *
  *************************************************************************** */
QPen PresentationS52::getPen(const FeatureS57 * feat) const
{
    if (!feat) throw "Internal Error: No Feature Pointer";
    QPen myPen;
    myPen.setCosmetic(true);

    //kai: just for debug, code not yet ready
    ++cnt; //++ *(const_cast<int*>(&cnt)); //rem: we are const
    myPen.setColor(colorVecDebug[cnt % 13]); //kai: just for debug
    

    return myPen;
}

//*****************************************************************************
/// Return the Brush to fill the shape of a area-geometry item
/*!
  *
  *************************************************************************** */
QBrush PresentationS52::getBrush(const FeatureS57 * feat) const
{
    //**** if no area-feat -> no brush needed ****
    if (feat->getFRID().getPRIM() != 3) return QBrush();

    const unsigned short objCode = feat->getFRID().getOBJL();

    //**** Group 1 feature ****
    if ( ObjAttrDictionaryS57::IsGroup1(objCode))
    {
        
        if (objCode == ObjAttrDictionaryS57::codeDEPARE || objCode == ObjAttrDictionaryS57::codeDRGARE)
        {
            QBrush depareBrush;
            const std::map<unsigned short, FieldAttr> & featAttribs = feat->getAttribs();
            double drval1, drval2;
            if(ObjAttrDictionaryS57::getDRVAL12(drval1, drval2, featAttribs))
            {
                if (drval1 < 0 || drval2 < 0) depareBrush = interTidalBrush;
                if (drval1 > deep)        depareBrush = depareDeepBrush;
                if (drval2 < veryShallow) depareBrush = depareVeryShallowBrush;
                if (drval2 < shallow)     depareBrush = depareShallowBrush;
                depareBrush = depareMediumBrush;
            }
            if (objCode == ObjAttrDictionaryS57::codeDRGARE)
            {
                depareBrush.setStyle(Qt::Dense1Pattern);
            }
            return depareBrush;
        }
        else if (objCode == ObjAttrDictionaryS57::codeUNSARE)
        {
            return UNSAREbrush;
        }
        else if (objCode == ObjAttrDictionaryS57::codeFLODOC)
        {
            return FLODOCbrush;
        }        
        else if (objCode == ObjAttrDictionaryS57::codeHULKES)
        {
            return HULKESbrush;
        }        
        else if (objCode == ObjAttrDictionaryS57::codeLNDARE)
        {
            return LNDAREbrush;
        }        
        else if (objCode == ObjAttrDictionaryS57::codePONTON)
        {
            return PONTONbrush;
        }
    }

    //**** Group 2 feature ****
    else
    {
        QBrush featBrush(Qt::SolidPattern); //rem: default brush is NO-Brush!
        QColor featColor;
        //** Coverage object: no brush if there is coverage **
        if (objCode == ObjAttrDictionaryS57::codeM_COVR)
        {
            int catCov;
            if (ObjAttrDictionaryS57::getIntVal(catCov, feat->getAttribs(), ObjAttrDictionaryS57::aCodeCATCOV) && catCov == 2) //no coverage
            {
                featColor = QColor(244,244,244); 
            }
            else //coverage
            {
                return QBrush();
            }
        }
        //** Feature Objects not yet handled -> use fantasy color **
        else
        {
            featColor = colorVecDebug[cnt % 13]; // for debug only: default fantasy color for Objects not yet in pres-lib: **   
        }
        featColor.setAlpha(70); //kai: test: Group2 tranparent ??
        featBrush.setColor(featColor); 
        return featBrush;
    }
    return QBrush();
}

//*****************************************************************************
/// Get the Drawing Priority of a feature
/*!
  * Objects with higher values will be drawn onto objects with lower value
  *************************************************************************** */
double PresentationS52::getPriority(const FeatureS57 * feat) const
{
    if (feat->getFRID().getPRIM() == 1) //Point
    {
        return 250; //Kai: here we should distinct between more and less important objects
    }
    else if (feat->getFRID().getPRIM() == 2) return 200; //Line
    else if (feat->getFRID().getPRIM() == 3) //area
    {
        if (ObjAttrDictionaryS57::IsGroup1(feat->getFRID().getOBJL())) return 0; 
        else
        {
            return 100;
        }
    }

    return 0; //or should we throw someting??
}
