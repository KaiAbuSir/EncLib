#ifndef DICTIONARYS52_H
#define DICTIONARYS52_H

#include <vector>
#include <map>

#include <QtCore/QString>
#include <QtGui/QPen>
#include <QtGui/QBrush>

namespace Enc
{
class FeatureS57;

//*****************************************************************************
/// Dictionay - just a dummy for now - will become a real dict later
/*!
  *
  *************************************************************************** */
class PresentationS52
{
public:
    PresentationS52();
    QPen getPen(const FeatureS57 * feat) const;
    QBrush getBrush(const FeatureS57 * feat) const;

private:
    int cnt;
    std::vector<QColor> colorVecDebug;  //just used to make chart colorfull for easier debugging

    //** group 1 Objects have predefined values **
    QBrush depareShallowBrush, depareNormalBrush, depareDeepBrush;
    QBrush lndareBrush;
};

//*****************************************************************************
/// Dictionay - just a dummy for now - will become a real dict later
/*!
  *
  *************************************************************************** */
class ObjAttrDictionaryS57
{
public:
    ObjAttrDictionaryS57();

    static bool IsGroup1(unsigned int objCode)  ///returns true if objCode is one of the 6 Group1 Objects
    {
        return (objCode == codeDEPARE || objCode == codeDRGARE || objCode == codeFLODOC || objCode == codeHULKES ||
                objCode == codeLNDARE || objCode == codePONTON || objCode == codeUNSARE);
    }

    QString getFeatToken4Code(unsigned int code) const;
    QString getAttrToken4Code(unsigned int code) const;

private:
    void readAttributeCodes(QString fileName);
    void readFeatureCodes(QString filename);


    std::map<unsigned int, QString> featureCode2Token;       //key = feature code
    std::map<QString, unsigned int> featureToken2Code;       //key = 6char-token
    std::map<unsigned int, QString> featureCode2Name;        //Full Name of Feature

    std::map<unsigned int, QString> attributeCode2Token;
    std::map<QString, unsigned int> attributeToken2Code;
    std::map<unsigned int, QString> attributeCode2Name;
public:

    //Group1 - most important object codes in s57
    static const unsigned short codeDEPARE = 42; //most important number in S57
    static const unsigned short codeDRGARE = 46;
    static const unsigned short codeFLODOC = 57;
    static const unsigned short codeHULKES = 65;
    static const unsigned short codeLNDARE = 71;
    static const unsigned short codePONTON = 95;
    static const unsigned short codeUNSARE = 154;
};

}

#endif // DICTIONARYS52_H
