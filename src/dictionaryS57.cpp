#include "dictionaryS57.h"

#include <QtCore/QFile>

#include <map>


#include "cell_records.h"

using namespace Enc;


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

//*****************************************************************************
/// Convenience Method to get DRVAL1 and DRVAL2
/*!
  * returns true only if both values are correct
  *************************************************************************** */
bool ObjAttrDictionaryS57::getDRVAL12(double & drval1, double & drval2, const std::map<unsigned short, FieldAttr> & attrMap)
{
    if (!getDoubleVal(drval1, attrMap, aCodeDRVAL1)) return false;
    if (!getDoubleVal(drval1, attrMap, aCodeDRVAL2)) return false;
    return true;
}

//*****************************************************************************
/// Returns the value of a double-attribute
/*!
  * returns false if attribute not found or conversion fails
  *************************************************************************** */
bool ObjAttrDictionaryS57::getDoubleVal(double & val, const std::map<unsigned short, FieldAttr> & attrMap, unsigned short key)
{
    std::map<unsigned short, FieldAttr>::const_iterator vIt = attrMap.find(key);
    if (vIt == attrMap.end()) return false;
    bool valOk;
    val = vIt->second.getValue().toDouble(&valOk);
    return valOk;
}

bool ObjAttrDictionaryS57::getIntVal(int & val, const std::map<unsigned short, FieldAttr> & attrMap, unsigned short key)
{
    std::map<unsigned short, FieldAttr>::const_iterator vIt = attrMap.find(key);
    if (vIt == attrMap.end()) return false;
    bool valOk;
    val = vIt->second.getValue().toInt(&valOk);
    return valOk;
}

//*****************************************************************************
///
/*!
  *
  *************************************************************************** */
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

//*****************************************************************************
///
/*!
  *
  *************************************************************************** */
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
