#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

#include "exsetview.h"
#include "exSetFilterWidget.h"
#include "iso8211_simple.h"
#include "catalog031reader.h"


using namespace Enc;

void showAllColumns(bool showAll);

//*****************************************************************************
/// Constructor - Creates GUI
/*!
*
****************************************************************************** */
ExSetView::ExSetView(QWidget * parent) : QTableView(parent)
{
    setSortingEnabled(true);
}

ExSetView::~ExSetView()
{}

//******************************************************************************
/// Show / hide some columns which are of minor importance to the user
/*!
***************************************************************************** */
void ExSetView::showAllColumns(bool showAll)
{
    if (showAll)
    {
        showColumn(0);
        showColumn(1);
        showColumn(2);
        showColumn(5);
    }
    else
    {
        hideColumn(0);
        hideColumn(1);
        hideColumn(2);
        hideColumn(5);
    }
}

//******************************************************************************
/// Apply the Filter Settings
/*!
***************************************************************************** */
void ExSetView::doFilter(ExSetFilterSettings newFilterSettings)
{
    if (currentFilterSettings == newFilterSettings)
    {

    }
    else
    {
        currentFilterSettings = newFilterSettings;
    }
}

//******************************************************************************
/// Remove all Filter Settings 
/*!
***************************************************************************** */
void ExSetView::unFilter()
{
    currentFilterSettings = ExSetFilterSettings();
}

//******************************************************************************
///
/*!
***************************************************************************** */
ExSetCheckWidget::ExSetCheckWidget(QWidget * parent) : QWidget(parent)
{
    QHBoxLayout * mainLyt = new QHBoxLayout(this);
    checkAllBtn = new QPushButton(tr("Check All"), this);
    checkSelBtn = new QPushButton(tr("Check Selected"), this);
    mainLyt->addStretch(1);
    mainLyt->addWidget(checkAllBtn, 0);
    mainLyt->addWidget(checkSelBtn, 0);

    connect(checkAllBtn, SIGNAL(clicked()), this, SIGNAL(checkAll()));
    connect(checkSelBtn, SIGNAL(clicked()), this, SIGNAL(checkSelected()));
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
///
/*!
***************************************************************************** */
ExSetModel::ExSetModel(QObject * parent)
          : QAbstractTableModel(parent)
{}
ExSetModel::~ExSetModel()
{}

void ExSetModel::init(const std::vector<CatalogEntry> & newCatCnt)
{
    catCont = newCatCnt;
    reset();
}

void ExSetModel::init(QString catFileName)
{
    Catalog031reader catReader;
    catReader.readCatalog031(catFileName, catCont);
    reset();
    /* instead of reset(), one could use this: 
    QModelIndex topLeft = index(0, 0, QModelIndex());
    QModelIndex bottomRight = index(catCont.size() -1, CatalogEntry::MemberCnt -1, QModelIndex());
    emit dataChanged(topLeft,bottomRight); 
    emit headerDataChanged( Qt::Vertical, 0, catCont.size() -1); //if headerData Signal is not emitted, data will not be displayed!!!*/
}

int ExSetModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;
    return catCont.size();
}

int ExSetModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;
    return 10;
}

//******************************************************************************
///
/*!
***************************************************************************** */
QVariant ExSetModel::data(const QModelIndex & index, int role) const
{
    //**** check if index is valid *****
    if (index.row() >= catCont.size() || index.column() >= CatalogEntry::MemberCnt) 
    {
        return QVariant();
    }
    //**** rem: Data-Role is handled by CatalogEntry-struct itself ****
    if (role == Qt::DisplayRole)
    {
        const CatalogEntry & catEntry = catCont[index.row()];
        return catEntry[index.column()];
    }
    //**** ToolTips: display column contents for columns which are typically long ****
    else if (role == Qt::ToolTipRole)
    {
        const CatalogEntry & catEntry = catCont[index.row()];
        if (index.column() == CatalogEntry::COL_LFIL)
        {
            if (!catEntry.LFIL.isEmpty()) return catEntry.LFIL;
        }
        else if (index.column() ==  CatalogEntry::COL_IMPL)
        {
            return S57::IMPL2string(catEntry.IMPL);
        }
        else if (index.column() ==  CatalogEntry::COL_CRCS)
        {
            if (catEntry.CRCS == 0) return tr("No Checksum");
            else
            {
                return QString("Checksum: 0x%1 = %2").arg(catEntry.CRCS, 8, 16, QChar('0')).arg(catEntry.CRCS, 10, 10, QChar('0'));
            }
        }
        else if (index.column() ==  CatalogEntry::COL_COMT)
        {
            if (!catEntry.COMT.isEmpty()) return catEntry.COMT;
        }
    }
    return QVariant();
}

//******************************************************************************
/// Get the header data for a catalog-hodel
/*!
***************************************************************************** */
QVariant ExSetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            if (section == CatalogEntry::COL_RECID8211) return "ISO 8211 RecordId";
            else if (section == CatalogEntry::COL_RCNM) return "Token";
            else if (section == CatalogEntry::COL_RCID) return "Record Id";
            else if (section == CatalogEntry::COL_FILE) return "File Name";
            else if (section == CatalogEntry::COL_LFIL) return "Long File Name";
            else if (section == CatalogEntry::COL_VOLM) return "Volume";
            else if (section == CatalogEntry::COL_IMPL) return "Impementation";
            else if (section == CatalogEntry::COL_bBox) return "Bounding Box";
            else if (section == CatalogEntry::COL_CRCS) return "Checksum";
            else if (section == CatalogEntry::COL_COMT) return "Comment";
            return QVariant();
        }
        else if (role == Qt::ToolTipRole)
        {
            if (section == CatalogEntry::COL_RECID8211)
            {
                return tr("ISO 8211 Record ID\nWill be set automatically\nIrrelevant for S-57");      
            }
            else if (section == CatalogEntry::COL_RCNM) 
            {
                return tr("S-57 Record Name Token, always \"CD\"");
            }
            else if (section ==  CatalogEntry::COL_RCID)
            {
                return tr("S-57 Record ID\nWill be set automatically");
            }
            else if (section ==  CatalogEntry::COL_FILE)
            {
                return tr("Filename including Path relative to Catalog file");
            }
            else if (section ==  CatalogEntry::COL_LFIL)
            {
                return tr("Long Filename, explaining the file contents to user");
            }
            else if (section ==  CatalogEntry::COL_VOLM)
            {
                return tr("Volume identifier, \"V01X01\" if all Files are on a single medium");
            }
            else if (section ==  CatalogEntry::COL_IMPL)
            {
                return tr("Implentation = Type of File: \n\"BIN\" = S-57 Base/Upadte \n\"ASC\" = Catalog File \n\"TXT\" = Text file \n... etc");
            }
            else if (section ==  CatalogEntry::COL_bBox)
            {
                return tr("Bounding Box, only relevant for Base/Update Files");
            }
            else if (section ==  CatalogEntry::COL_CRCS)
            {
                return tr("Checksum");
            }
            else if (section ==  CatalogEntry::COL_COMT)
            {
                return tr("Comment, used in S-63 Exchange Sets to Store: \nEdition \nUpdate \nIssueDate \nUpdateDate");
            }
        }
    }
    //**** rem: No need for (orientation == Qt::Horizontal) header, because row-numbers are provided by QAbstractTableModel 
    return QAbstractTableModel::headerData(section, orientation,role);
}

//******************************************************************************
///
/*!
***************************************************************************** */
ExSetSortFilterModel::ExSetSortFilterModel(QObject * parent) : QSortFilterProxyModel(parent) 
{
    
}

bool ExSetSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
{
    //QModelIndex sourceIndex = sourceModel()->index(sourceRow, CatalogEntry::COL_FILE, sourceParent);
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool ExSetSortFilterModel::lessThan(const QModelIndex & left, const QModelIndex & right)const
{
    if (left.column() == CatalogEntry::COL_bBox && right.column() == CatalogEntry::COL_bBox)
    {
        if (left.data().type() == QVariant::String && right.data().type() == QVariant::String)
        {
     	    DegBBox leftBB(left.data().toString());
            DegBBox rightBB(right.data().toString());
            return leftBB < rightBB;
        }
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

void ExSetSortFilterModel::filterFileName(QString regExp)
{
    setFilterKeyColumn(CatalogEntry::COL_FILE);  //used for filtering filename

}