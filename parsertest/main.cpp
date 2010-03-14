#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QStringList>

#include "cell_s57.h"
#include "s57updater.h"
#include "cell_check_s57.h"

using namespace Enc;

int main( int argc, char ** argv)
{
    if (argc <2)
    {
        printf("\nError: no input dir\n");
        exit(1);
    }
    //**** parse input dir ****

    QString baseFile;
    QStringList updateFiles;
    QDir inDir(argv[1]);
    QStringList entries = inDir.entryList(QDir::Files);
    for (QStringList::iterator dIt = entries.begin(); dIt != entries.end(); ++dIt)
    {
        if ((*dIt).right(4) == ".000") baseFile = *dIt;
        else if ((*dIt).right(3).toUInt() >= 1 && (*dIt).right(3).toUInt() <= 999) 
        {
            updateFiles += inDir.absoluteFilePath(*dIt);
        }
    }

    //**** read a S-57 cell ****
    CellS57_Base s57cell;
    try
    {
        s57cell.parseISO8211(inDir.absoluteFilePath(baseFile).toLatin1());
    }
    catch(const QString & msg)
    {
        qWarning(msg.toLatin1());
        exit(3);
    }
    catch(...)
    {
        printf("UNKNOWN ERROR while reading: %s", argv[1]);
        exit(3);
    }

    //**** check cell after parsing: ****
    CellCheckS57 cellChecker(&s57cell);
    CheckResult checkResult = cellChecker.check();
    if (checkResult.errCnt() || checkResult.outsidePointer.size() )
    {
        if (checkResult.errCnt()) qWarning("ERROR: %d errros after cell parsing:", checkResult.errCnt());
        QStringList errMsg;
        checkResult.makeMessage(errMsg);
        printf("\n%s", errMsg.join("\n").toLatin1());
    }

    /*const std::multimap < LongNAMe, unsigned long > foids = s57cell.getFoids();
    std::multimap < LongNAMe, unsigned long >::const_iterator fIt;
    for (fIt = foids.begin(); fIt != foids.end(); ++fIt)
    {
        printf("\n%s %s", ISO8211::makeRecNameASCII(RCNM_FE, fIt->second).toLatin1().data(), ISO8211::makeLongNAMeASCII(fIt->first).toLatin1().data());
    } */

    //**** read and apply update, if available ****
    try
    {
        if (!updateFiles.empty())
        {
            s57cell.applyUpdates(updateFiles);
        }
    }
    catch(const QString & msg)
    {
        qWarning(msg.toLatin1());
        exit(4);
    }

    //**** write the cell to file ****
    try
    {
        if (argc >= 1)
        {
            QDir outDir(argv[2]);
            s57cell.writeISO8211(outDir.absoluteFilePath(baseFile).toLatin1());
        }
    }
    catch(const QString & msg)
    {
        qWarning(msg.toLatin1());
        exit(3);
    }
    catch(...)
    {
        printf("UNKNOWN ERROR while reading: %s", argv[1]);
        exit(3);
    }
    return 0;
}