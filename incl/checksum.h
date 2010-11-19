#ifndef CheckSum_H
#define CheckSum_H

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QTableWidget>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QMutex>


namespace Enc
{

//******************************************************************************
/// Class calculation a CRC (Checksum) for a file or a byte-field
/*!
* Algorithm is slightly optimized, but not the fastest.
***************************************************************************** */
class CheckSum
{
public:
    CheckSum();
    ~CheckSum();
    unsigned int calcCRC(QString fileName);
    unsigned int calcCRC(const unsigned char *, int len);

private:
    void init();
    unsigned int crctab[256];

    char * myData; //Used in case memory is allocated by this class
    int myDataLen; //size of myData
    void reAlloc(int newDataLen);
};

//******************************************************************************
/// Class calculating CRCs (Checksums) for several file in a thread
/*!
* Results are emitted by Signals, only
***************************************************************************** */
class CheckSumThread : public QThread
{
    Q_OBJECT
signals:
    void checksum(int, unsigned int);
    void checksumError(int, QString);

public:
    CheckSumThread();
    void calcCRCs(QStringList fNames);

public slots:
    void cancel();

protected:

    virtual void run();
    QMutex cancelMtx;
    bool cancelSoon;
    QStringList fileNames;
};

//******************************************************************************
/// Dialog to select files/directories for CRC calculation
/*!
* Dialog must be called NON-MODAL - Dialog deletes itself on close !!!
***************************************************************************** */
class CheckSumDialog : public QDialog
{
    Q_OBJECT
public:
    CheckSumDialog(QWidget * parent =0);
    virtual ~CheckSumDialog();

private slots:
    void onOpenFiles();
    void onOpenDir();
    void onCancel();
    void onChecksum(int, unsigned int);
    void onError(int, QString);
    void onCalcFinished();

private:

    void setFileNames2Table(QStringList fileNames, int & row);
    void getFileNamesFromTable(QStringList & realFiles);
    void calcCRCs(QStringList files);
 
    QLineEdit * fileNameEdt;
    QTableWidget * crcTbl;
    QProgressBar * progBar;
    QPushButton * cancelBtn;

    CheckSumThread crcCalculon;

    double progVal, progStep;
};
}
#endif

