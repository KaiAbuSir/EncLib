#ifndef DICTIONARY_S57_H
#define DICTIONARY_S57_H

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
/// Dictionary - holds codes, Tokens ... etc of Features and Attributes
/*!
  * Just a dummy until now
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

    static bool getDRVAL12(double & drval1, double & drval2, const std::map<unsigned short, FieldAttr> & attrMap);
    static bool getDoubleVal(double & val, const std::map<unsigned short, FieldAttr> & attrMap, unsigned short key);
    static bool getIntVal(int & val, const std::map<unsigned short, FieldAttr> & attrMap, unsigned short key);

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

    //Group2 - Other important object codes is s57
    static const unsigned short codeM_COVR = 302;

    //** important Attribute Codes in S57:
    static const unsigned short aCodeCATCOV = 18;
    static const unsigned short aCodeDRVAL1 = 87;
    static const unsigned short aCodeDRVAL2 = 88;
    
};

}

#endif // DICTIONARYS52_H
