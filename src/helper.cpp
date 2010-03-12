#include "helper.h"

using namespace Enc;

//******************************************************************************
/// Write absolute filenames into a StringList, and go recursive into subdirectories, too
/*!
***************************************************************************** */
void Helper::absFileNames2List(QStringList filesAndDirs, QStringList & nameList)
{
    QStringList::iterator fit = filesAndDirs.begin();
    for (; fit != filesAndDirs.end(); ++fit)
    {
        if (QFileInfo(*fit).isDir())
        {
            if ((*fit).isEmpty() || *fit == "." || *fit == "..") continue;
            QDir subDir(*fit);
            QStringList entryList = subDir.entryList();
            for (QStringList::iterator eit = entryList.begin(); eit != entryList.end(); ++eit)
            {
                if ((*eit).isEmpty() || *eit == "." || *eit == "..") continue;
                
                *eit = subDir.absoluteFilePath(*eit); //turn every entry into an absolut path
                QString c2 = (*eit);
            }
            absFileNames2List(entryList, nameList);
        }
        else
        {
            nameList.append(*fit);
        }
    }
}
