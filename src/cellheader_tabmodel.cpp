#include "cellheader_tabmodel.h" 
#include "cell_records.h"
#include "cell_parser_iso8211dirty4header.h"

//#include <QtGui/>
#include <QtCore/QDir>

using namespace Enc;

HeaderTableModel::HeaderTableModel(QObject * parent) : QAbstractTableModel(parent)
{}
HeaderTableModel::~HeaderTableModel()
{}

void HeaderTableModel::init(const std::vector<CellS57_Header> & cellsContent)
{
    cells = cellsContent;
    reset();
}

void HeaderTableModel::init(QStringList cellFileList)
{
    CellParser8211Dirty4Header clHdrRdr;
    QStringList::const_iterator cit = cellFileList.begin();
    for(; cit != cellFileList.end(); ++cit)
    {
        try
        {
            CellS57_Header cell;
            clHdrRdr.setCell(&cell);
            clHdrRdr.parseS57Cell(*cit);
            cells.push_back(cell);
        }
        catch (const char * errMsg)
        {
            printf("DEBUG: Error parsing cell: %s - %s",(*cit).toLatin1(), errMsg);
        }
    }
    reset();
}

int HeaderTableModel::rowCount(const QModelIndex & parent) const
{
    return cells.size();
}
int HeaderTableModel::columnCount(const QModelIndex & parent ) const
{
    return  FieldDSID::MemberCnt;
}
QVariant HeaderTableModel::data(const QModelIndex & index, int role) const
{
    //**** check if index is valid *****
    if (index.row() >= cells.size() || index.column() >= FieldDSID::MemberCnt) 
    {
        return QVariant();
    }
    //**** rem: Data-Role is handled by CatalogEntry-struct itself ****
    if (role == Qt::DisplayRole)
    {
        const FieldDSID & headerData = cells[index.row()].getDsid();
        if (index.column() == FieldDSID::COL_EXPP)
        {
            return S57::EXPP2string(headerData.getEXPP());
        }
        else if (index.column() == FieldDSID::COL_INTU)
        {
            return S57::INTU2string(headerData.getINTU());
        }
        else if (index.column() == FieldDSID::COL_PROF)
        {
            return S57::PROF2string(headerData.getPROF());
        }
        return headerData[index.column()];
    }
    //**** ToolTips: display column contents for columns which are typically long ****
    else if (role == Qt::ToolTipRole)
    {
        const FieldDSID & dsid = cells[index.row()].getDsid();
        if (index.column() == FieldDSID::COL_DSNM)
        {
            if (!dsid.getDSNM().isEmpty()) return dsid.getDSNM();
        }
        else if (index.column() == FieldDSID::COL_COMT)
        {
            if (!dsid.getCOMT().isEmpty()) return dsid.getCOMT();
        }
    }
    return QVariant();
}
QVariant HeaderTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            if      (section == FieldDSID::COL_RCNM) return tr("Token");
            else if (section == FieldDSID::COL_RCID) return tr("Record Id");
            else if (section == FieldDSID::COL_EXPP) return tr("Exchange purpose");
            else if (section == FieldDSID::COL_INTU) return tr("Usage");
            else if (section == FieldDSID::COL_DSNM) return tr("Dataset name");
            else if (section == FieldDSID::COL_EDTN) return tr("Edition");
            else if (section == FieldDSID::COL_UPDN) return tr("Update Number");
            else if (section == FieldDSID::COL_UADT) return tr("Update Date");
            else if (section == FieldDSID::COL_ISDT) return tr("Issue Date");
            else if (section == FieldDSID::COL_STED) return tr("S-57 Ed. Num.");
            else if (section == FieldDSID::COL_PRSP) return tr("Prod. Spec.");
            else if (section == FieldDSID::COL_PSDN) return tr("Prod. descr.");
            else if (section == FieldDSID::COL_PRED) return tr("Prod. Spec. ed.");
            else if (section == FieldDSID::COL_PROF) return tr("App. profile");
            else if (section == FieldDSID::COL_AGEN) return tr("Ag Code");
            else if (section == FieldDSID::COL_COMT) return tr("Comment");
            return QVariant();
        }
        else if (role == Qt::ToolTipRole)
        {
            if (section == FieldDSID::COL_RCNM) 
            {
                return tr("S-57 Record Name Token, always \"CD\"");
            }
            else if (section ==  FieldDSID::COL_RCID)
            {
                return tr("S-57 Record ID");
            }
            else if (section ==  FieldDSID::COL_EXPP)
            {
                return tr("Exchange purpose, normally: \n- 'N' (new) or \n'R' (revision)");
            }
            else if (section ==  FieldDSID::COL_DSNM)
            {
                return tr("Dataset name");

            }
            else if (section ==  FieldDSID::COL_UPDN)
            {
                return tr("Update Number");
            }
            else if (section ==  FieldDSID::COL_UADT)
            {
                return tr("Update Application Date \nOnly for Reissues");
            }
            else if (section ==  FieldDSID::COL_ISDT)
            {
                return tr("Issue Date");
            }
            else if (section ==  FieldDSID::COL_STED)
            {
                return tr("S-57 Edition Number \nAlways \"03.1\" for S-57 ENCs");
            }
            else if (section ==  FieldDSID::COL_PRSP)
            {
                return tr("Product Specification \nnormally \"ENC\" for S-57 Naviagtion Charts");
            }
            else if (section ==  FieldDSID::COL_PSDN)
            {
                return tr("Product Specification description");
            }
            else if (section ==  FieldDSID::COL_PRED)
            {
                return tr("Product Specification edition number");
            }
            else if (section ==  FieldDSID::COL_PROF)
            {
                return tr("Application profile identification - \"EN\" \"ER\" or \"DD\"");
            }
            else if (section ==  FieldDSID::COL_AGEN)
            {
                return tr("Agency Code of Producing Agency");
            }
            else if (section ==  FieldDSID::COL_COMT)
            {
                return tr("Comment: ");
            }
        }
    }
    //**** rem: No need for (orientation == Qt::Horizontal) header, because row-numbers are provided by QAbstractTableModel 
    return QAbstractTableModel::headerData(section, orientation,role);

}

