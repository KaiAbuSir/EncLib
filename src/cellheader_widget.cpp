#include "cellheader_widget.h" 
#include "cell_records.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>
//#include <QtGui/>
#include <QtCore/QDir>

using namespace Enc;

//******************************************************************************
/// 
/*!
***************************************************************************** */
HeaderTableView::HeaderTableView(QWidget *parent) : QTableView(parent)
{
}
HeaderTableView::~HeaderTableView()
{
}
void HeaderTableView::showAllColumns(bool showAll)
{
    if (showAll)
    {
        showColumn(FieldDSID::COL_RCNM);
        showColumn(FieldDSID::COL_RCID);
        showColumn(FieldDSID::COL_STED);
        showColumn(FieldDSID::COL_PRSP);
        showColumn(FieldDSID::COL_PSDN);
        showColumn(FieldDSID::COL_PRED);
    }
    else
    {
        hideColumn(FieldDSID::COL_RCNM);
        hideColumn(FieldDSID::COL_RCID);
        hideColumn(FieldDSID::COL_STED);
        hideColumn(FieldDSID::COL_PRSP);
        hideColumn(FieldDSID::COL_PSDN);
        hideColumn(FieldDSID::COL_PRED);
    }
}

//******************************************************************************
/// Constructor: Create widgets and make connection to headerReader-thread
/*!
***************************************************************************** */
CellHeaderDialog::CellHeaderDialog(QWidget * parent) : QDialog(parent), headerTableModel(0)
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

    headerDataTbl = new HeaderTableView(this);   
    mainLyt->addWidget(headerDataTbl, 1000); 

    QPushButton * allBtn = new QPushButton("Show All", this);
    allBtn->setCheckable(true);
    mainLyt->addWidget(allBtn, 0);

    headerTableModel = new HeaderTableModel(this);
    headerDataTbl->setModel(headerTableModel);

    QHBoxLayout * progLyt = new QHBoxLayout(this);
    mainLyt->addLayout(progLyt, 0);
    progBar = new QProgressBar(this);
    cancelBtn = new QPushButton(tr("Cancel"), this);
    progLyt->addWidget(progBar, 1); 
    progLyt->addWidget(cancelBtn, 0); 

    //**** inits ****
    headerDataTbl->showAllColumns(allBtn->isDown());

    //**** connections ****
    connect(openFileBtn, SIGNAL(clicked()), this, SLOT(onOpenFiles()));
    connect(openDirBtn, SIGNAL(clicked()), this, SLOT(onOpenDir()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(onCancel()));

    connect(allBtn, SIGNAL(toggled(bool)), headerDataTbl, SLOT(showAllColumns(bool)));
    //** rem: the following connections carry data between different threads **
    //connect(&crcCalculon, SIGNAL(checksum(int, unsigned int)), this, SLOT(onChecksum(int, unsigned int)), Qt::QueuedConnection);
    //connect(&crcCalculon, SIGNAL(checksumError(int, QString)), this, SLOT(onError(int, QString)), Qt::QueuedConnection);
    //connect(&crcCalculon, SIGNAL(finished()), this, SLOT(onCalcFinished()));
}


//******************************************************************************
/// Destructor - deletes the Dialog itself -> dont delete Dialog manually!!
//***************************************************************************** */
CellHeaderDialog::~CellHeaderDialog()
{
    delete headerTableModel;
    headerTableModel = NULL;
    if (!isModal()) this->deleteLater();
}

//******************************************************************************
/// Get a directory to pares all included cells, including subdirs
/*!
* all Subdirs of "dir" are evaluated recursively !
***************************************************************************** */
void CellHeaderDialog::onOpenDir()
{
    QDir recentDir(fileNameEdt->text());
    if (!recentDir.exists()) recentDir = QDir::home();
    
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory to parse all included cells"), recentDir.absolutePath(), QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;
    
    fileNameEdt->setText(dir);
   
    //fehlt: cell-parsen 
}

//******************************************************************************
/// Get filenames of cells to parse 
/*!
***************************************************************************** */
void CellHeaderDialog::onOpenFiles()
{
    QDir recentDir(fileNameEdt->text());
    if (!recentDir.exists()) recentDir = QDir::home();
    QStringList files = QFileDialog::getOpenFileNames(this,  tr("Select one or more cells to read"), recentDir.currentPath(), tr("All files (*);;S-57 Chart (*.000)"));
    if (files.isEmpty()) return;
    
    paresCells(files); 
}

//******************************************************************************
/// Init the calculation of all CRCs in "files"
/*!
* before calculating, all Filenames are set to table - that can take much time :-(
* The actual calculation is done by crcCalculon in a separate Thread
***************************************************************************** */
void CellHeaderDialog::paresCells(QStringList files)
{
    if (files.count() == 0) return;
    else if (files.count() == 1) fileNameEdt->setText(files.first());
    else
    {
        fileNameEdt->setText(QFileInfo(files.first()).absoluteDir().absolutePath());
    }
    headerTableModel->init(files);
}
