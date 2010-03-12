#include <QtCore/QMutexLocker>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QMessageBox>
#include "checksum.h"

using namespace Enc;

#define QUOTIENT 0x04c11db7  //needed for fast CRC calculation

CheckSum::CheckSum() : myData(0), myDataLen(0)
{
    init();
}

CheckSum::~CheckSum()
{
    delete [] myData;
}

//******************************************************************************
/// Calcualte some factors needed for fast CRC calculation  
//**************************************************************************** */
void CheckSum::init()
{
    int i, j;

    unsigned int crc;

    for (i = 0; i < 256; i++)
    {
        crc = i << 24;
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x80000000)
                crc = (crc << 1) ^ QUOTIENT;
            else
                crc = crc << 1;
        }
        crctab[i] = crc;
    }
}

//******************************************************************************
/// ReAllocate memory, but only if necessary 
//***************************************************************************** */
void CheckSum::reAlloc(int newDataLen)
{
    if (myData && myDataLen < newDataLen) 
    {
        delete [] myData;
        myData = 0;
        myDataLen = 0;
    }
    if (myData == 0)
    {
        myData = new char[newDataLen];
        myDataLen = newDataLen;
    }
}

//******************************************************************************
/// Calculate the CRC of a file - throws exceptions on error !
//***************************************************************************** */
unsigned int CheckSum::calcCRC(QString fileName)
{
    QFile crcF(fileName);
    if (!crcF.open(QIODevice::ReadOnly))
    {
        throw  QString("ERROR: cannot open File: ") +fileName;  
    }
    int fSize = crcF.size();
    reAlloc(fSize);
    qint64 realSize = crcF.read(myData, fSize);
    if (fSize != realSize)
    {
        throw  QString("ERROR: cannot read File completely: ") +fileName;  
    }
    crcF.close();
    unsigned int crc = calcCRC((unsigned char *)myData, fSize);
    return crc;
}

//******************************************************************************
/// Calculate the CRC of a byte field
/*!
* Algorithm is somehow optimized - there are faster algorithms, but with copyright :-(
***************************************************************************** */
unsigned int CheckSum::calcCRC(const unsigned char *data, int len)
{
    unsigned int        result;
    int                 i;
   
    if (len < 4) return 0;

    result = *data++ << 24;
    result |= *data++ << 16;
    result |= *data++ << 8;
    result |= *data++;
    result = ~ result;
    len -=4;
    
    for (i=0; i<len; i++)
    {
        result = (result << 8 | *data++) ^ crctab[result >> 24];
    }
    
    return ~result;
}

CheckSumThread::CheckSumThread() : cancelSoon(false)
{}

void CheckSumThread::cancel()
{
    QMutexLocker mLck(&cancelMtx);
    cancelSoon = true;
}

//******************************************************************************
/// Set the source-data and start the calculating thread
/*!
***************************************************************************** */
void CheckSumThread::calcCRCs(QStringList fNames)
{
    cancelSoon = false;
    fileNames = fNames;
    start();
}

//******************************************************************************
/// Calculate the Checksums and send it by signals to the calling thread
/*!
***************************************************************************** */
void CheckSumThread::run()
{
    CheckSum checkSumCalc;
    QStringList::iterator sit = fileNames.begin();
    for (int index = 0; sit != fileNames.end(); ++sit, ++index)
    {
        unsigned int crc = 0;
        try
        {
            crc = checkSumCalc.calcCRC(*sit);
        }
        catch(...)
        {
            emit checksumError(index, "ERROR");
            continue;
        }
        emit checksum(index, crc);
        {
            QMutexLocker mLck(&cancelMtx);
            if (cancelSoon) break;
        }
    }
}
     
//******************************************************************************
/// Constructor: Create widgets and make connection to calculator-thread
/*!
***************************************************************************** */
CheckSumDialog::CheckSumDialog(QWidget * parent) : QDialog(parent)
{
    QVBoxLayout * mainLyt = new QVBoxLayout(this);
    QHBoxLayout * openLyt = new QHBoxLayout(this);
    mainLyt->addLayout(openLyt);

    fileNameEdt = new QLineEdit(this);
    QPushButton * openFileBtn = new QPushButton(tr("Open File"), this);
    QPushButton * openDirBtn = new QPushButton(tr("Open Dir"), this);
    openLyt->addWidget(new QLabel(tr("File Name"), this), 0);
    openLyt->addWidget(fileNameEdt, 1);
    openLyt->addWidget(openFileBtn, 0);
    openLyt->addWidget(openDirBtn, 0);

    crcTbl = new QTableWidget(1,3,this);
    crcTbl->setHorizontalHeaderLabels(QStringList() << QString("Filename") << QString("Checksum Hex") << QString("Checksum Dec"));
    mainLyt->addWidget(crcTbl, 1000); 

    QHBoxLayout * progLyt = new QHBoxLayout(this);
    mainLyt->addLayout(progLyt, 0);
    progBar = new QProgressBar(this);
    cancelBtn = new QPushButton(tr("Cancel"), this);
    progLyt->addWidget(progBar, 1); 
    progLyt->addWidget(cancelBtn, 0); 

    connect(openFileBtn, SIGNAL(clicked()), this, SLOT(onOpenFiles()));
    connect(openDirBtn, SIGNAL(clicked()), this, SLOT(onOpenDir()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(onCancel()));

    //** rem: the following connections carry data between different threads **
    connect(&crcCalculon, SIGNAL(checksum(int, unsigned int)), this, SLOT(onChecksum(int, unsigned int)), Qt::QueuedConnection);
    connect(&crcCalculon, SIGNAL(checksumError(int, QString)), this, SLOT(onError(int, QString)), Qt::QueuedConnection);

    connect(&crcCalculon, SIGNAL(finished()), this, SLOT(onCalcFinished()));
}

//******************************************************************************
/// Destructor - deletes the Dialog itself -> dont delete Dialog manually!!
//***************************************************************************** */
CheckSumDialog::~CheckSumDialog()
{
    if (!isModal()) this->deleteLater();
}

//******************************************************************************
/// Get a directory to calcualte all CRCs of the included files
/*!
* all Subdirs of "dir" are evaluated recursively !
***************************************************************************** */
void CheckSumDialog::onOpenDir()
{
    QDir recentDir(fileNameEdt->text());
    if (!recentDir.exists()) recentDir = QDir::home();
    
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory to calculate the Checksum of all its files"), recentDir.absolutePath(), QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;
    
    fileNameEdt->setText(dir);
    calcCRCs(QStringList(dir));
}

//******************************************************************************
/// Get all filenames to calculate CRCs for
/*!
***************************************************************************** */
void CheckSumDialog::onOpenFiles()
{
    QDir recentDir(fileNameEdt->text());
    if (!recentDir.exists()) recentDir = QDir::home();
    QStringList files = QFileDialog::getOpenFileNames(this,  tr("Select one or more files to calculate the Checksum"), recentDir.currentPath(), tr("All files (*);;S-57 Chart (*.000)"));
    if (files.isEmpty()) return;
    calcCRCs(files);
}

//******************************************************************************
/// Init the calculation of all CRCs in "files"
/*!
* before calculating, all Filenames are set to table - that can take much time :-(
* The actual calculation is done by crcCalculon in a separate Thread
***************************************************************************** */
void CheckSumDialog::calcCRCs(QStringList files)
{
    fileNameEdt->setText(*files.begin());
    crcTbl->setRowCount(0);
    progBar->reset();
    int row =0;
    setFileNames2Table(files, row);
    crcTbl->setRowCount(row); //rem: setFileNames2Table()may allocate more rows than needed!
    crcTbl->resizeColumnToContents(0);
    
    QStringList realFiles;
    getFileNamesFromTable(realFiles); //rem files may contains directories

    progBar->reset();
    if (crcTbl->rowCount() == 0) return;
    progVal = 0;
    progStep = 100.0 / crcTbl->rowCount();
 
    crcCalculon.calcCRCs(realFiles); //here, a thread start , method returns immediately!
}

//******************************************************************************
/// Get all filenames from Table (get all first column entries)
/*!
***************************************************************************** */
void CheckSumDialog::getFileNamesFromTable(QStringList & realFiles)
{
    for (int row =0; row < crcTbl->rowCount(); ++row)
    {
        QTableWidgetItem * myItem = crcTbl->item (row, 0);
        if (!myItem || myItem->text().isEmpty()) realFiles.push_back(""); //should never happen
        else realFiles.push_back(myItem->text());
    }
}

//******************************************************************************
/// Write all filenames in "files" into crc-Tables, and go recursive into directories, if needed
/*!
* This might take long time - therefore, processEvents() is called often
***************************************************************************** */
void CheckSumDialog::setFileNames2Table(QStringList files, int & row)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)); 

    if (crcTbl->rowCount() < row + files.size()) crcTbl->setRowCount(crcTbl->rowCount() + files.size());
    QStringList::iterator fit = files.begin();
    for (; fit != files.end(); ++fit)
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
            setFileNames2Table(entryList, row);
        }
        else
        {
            crcTbl->setItem(row,0, new QTableWidgetItem(*fit));
            ++row;
            QCoreApplication::processEvents(); //repaint();  
        }
    }
    QApplication::restoreOverrideCursor(); 
}

//*****************************************************************************
/// Set Checksum for one File (hex and decimal)
//***************************************************************************** 
void CheckSumDialog::onChecksum(int row, unsigned int crc)
{
    if (crc && row < crcTbl->rowCount())
    {
        crcTbl->setItem (row, 1, new QTableWidgetItem(QString("0x%1").arg(crc,8,16,QChar('0'))));
        crcTbl->setItem (row, 2, new QTableWidgetItem(QString::number(crc,10)));
    }
    //**** ensure that item is visible every 10 lines ****
    if (row %15 == 0 && (row +15) < crcTbl->rowCount())
    {
        QTableWidgetItem * item = crcTbl->item(row +15, 0);
        if (item) crcTbl->scrollToItem(item);
    }
    progVal += progStep;
    progBar->setValue(progVal);
}

//******************************************************************************
/// Inform user that calculation has failed for one file 
//***************************************************************************** 
void CheckSumDialog::onError(int row, QString errMsg)
{
    if (row < crcTbl->rowCount())
    {
        if (errMsg.isEmpty()) errMsg = "ERROR";
        QTableWidgetItem * errItem = new QTableWidgetItem(errMsg);
        errItem->setBackground(QBrush(QColor(255,0,0)));
        crcTbl->setItem (row, 1, errItem);
    }
    progVal += progStep;
    progBar->setValue(progVal);
}

//******************************************************************************
/// Inform user that calculation has finished (but only if it took long time)
//***************************************************************************** 
void CheckSumDialog::onCalcFinished()
{
    if (crcTbl->rowCount() > 500) QMessageBox::information(this, "CRC Calculation", "Calculation Finished");
}

//******************************************************************************
/// Set the cancel-flat so that the thread will stop as soon as possible (but not now)
//***************************************************************************** 
void CheckSumDialog::onCancel()
{
    crcCalculon.cancel();
}
 