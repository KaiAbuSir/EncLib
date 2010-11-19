#ifndef ExSetCheckS57_H
#define ExSetCheckS57_H

#include <QtCore/QString>

#include <vector>
#include <set>

namespace Enc
{
struct CatalogCheckEntry;

struct CheckErrors
{
    int missingFilesInDir;
    int missingFilesInCat;
    int CRCwrong;
    int recordErrors;  //missing bbox, missing volume ...

    CheckErrors() : missingFilesInDir(0), missingFilesInCat(0), CRCwrong(0), recordErrors(0)
    {}
};

class ExSetCheckS57
{
public:

    ExSetCheckS57();
    ~ExSetCheckS57();
    void init(QString catFileAndPath, std::vector<CatalogCheckEntry> * catEntries);
    void checkAll();
    void checkSome(std::set<int> indices2check);
    void checkEntry(CatalogCheckEntry &);

protected:

    QString catFilePath;
    std::vector<CatalogCheckEntry> * checkEntries;

    CheckErrors checkErrors;
};

}
#endif

