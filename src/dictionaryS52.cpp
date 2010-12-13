#include "dictionaryS52.h"

#include <QtCore/QFile>

#include <map>


#include "cell_records.h"

using namespace Enc;

//*****************************************************************************
/// Dictionary - just a dummy, will become a real dict later
/*!
  *
  *************************************************************************** */
PresentationS52::PresentationS52() : cnt(0)
{
    //** Brushes for Group1 Objects **
    depareShallowBrush = QBrush(QColor(187,241,251));   //kais hell blau, nur zum testen
    depareNormalBrush = QBrush(QColor(27,137,217));     //kais mittelblau, nur zum testen
    depareDeepBrush = QBrush(QColor(18,47,146));        //kais dunkelblau, nur zum testen

    lndareBrush = QBrush(QColor(231,164,24)); //kais braun, nur zum testen




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

    QBrush featBrush;

    //** for debug only: default fantasy color for Objects not yet in pres-lib: **
    QColor dgbColor = colorVecDebug[cnt % 13];
    dgbColor.setAlpha(100);
    featBrush.setColor(dgbColor); 

    //**** Group 1 feature ****
    if ( ObjAttrDictionaryS57::IsGroup1(objCode))
    {
        if (objCode == ObjAttrDictionaryS57::codeDEPARE)
        {
            const std::map<unsigned short, FieldAttr> & featAttribs = feat->getAttribs();

        }
        else if (objCode == ObjAttrDictionaryS57::codeDRGARE)
        {

        }
    }

    //**** Group 2 feature ****
    else
    {
        
        
    }
    return featBrush;
}


//*****************************************************************************
/// Constructor initializes Attribute/Feature code/token values
/*!
  * Default values are inside app
  *************************************************************************** */
ObjAttrDictionaryS57::ObjAttrDictionaryS57()
{
    readAttributeCodes(":/resources/Attributes.S57.txt");
    readFeatureCodes(":/resources/Features.S57.txt");
}

//*****************************************************************************
///
/*!
  *
  *************************************************************************** */
QString ObjAttrDictionaryS57::getFeatToken4Code(unsigned int code) const
{
    std::map<unsigned int, QString>::const_iterator it = featureCode2Token.find(code);
    if (it != featureCode2Token.end()) return it->second;
    return QString::null;
}

QString ObjAttrDictionaryS57::getAttrToken4Code(unsigned int code) const
{
    std::map<unsigned int, QString>::const_iterator it = attributeCode2Token.find(code);
    if (it != attributeCode2Token.end()) return it->second;
    return QString::null;
}

void ObjAttrDictionaryS57::readAttributeCodes(QString fileName)
{
    QFile attrFile(fileName);
    if (!attrFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw "Cannot open Attribute Codes File: " +fileName;
    }
    QTextStream stream(&fileName);
    int missCnt =0;
    for (int lineNum =0;!stream.atEnd(); ++lineNum)
    {
        QString line = stream.readLine(32000).trimmed();
        if (line.isEmpty() || line.at(0) == '#') continue;
        QStringList lineLst = line.split(',');
        if (lineLst.size() < 2)
        {
            ++missCnt;
            continue; //harmless Error: Token missing
        }
        bool intOk;
        unsigned int code = lineLst[0].toInt(&intOk);
        if (!intOk)
        {
            continue;
        }
        featureCode2Token[code] = lineLst[1].trimmed();
        featureToken2Code[lineLst[1].trimmed()] = code;
        if (lineLst.size() >= 3) featureCode2Name[code] = lineLst[2].trimmed();
    }
    attrFile.close();
}

void ObjAttrDictionaryS57::readFeatureCodes(QString fileName)
{
    QFile featFile(fileName);
    if (!featFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw "Cannot open Attribute Codes File: " +fileName;
    }
    QTextStream stream(&fileName);
    int missCnt =0;
    for (int lineNum =0;!stream.atEnd(); ++lineNum)
    {
        QString line = stream.readLine(32000).trimmed();
        if (line.isEmpty() || line.at(0) == '#') continue;
        QStringList lineLst = line.split(',');
        if (lineLst.size() < 2)
        {
            ++missCnt;
            continue; //harmless Error: Token missing
        }
        bool intOk;
        unsigned int code = lineLst[0].toInt(&intOk);
        if (!intOk)
        {
            continue;
        }
        attributeCode2Token[code] = lineLst[1].trimmed();
        attributeToken2Code[lineLst[1].trimmed()] = code;
        if (lineLst.size() >= 3) attributeCode2Name[code] = lineLst[2].trimmed();
    }
    featFile.close();
}
