#ifndef CELL_HEADER_TAB_MODEL_H
#define CELL_HEADER_TAB_MODEL_H

#include <QtGui/QTableWidget>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtCore/QString>
#include <QtCore/QAbstractTableModel>
#include <QtGui/QSortFilterProxyModel>

#include "cell_s57.h"

namespace Enc
{
class CellHeaderReader;
class HeaderTableView;
class FieldDSID;

//******************************************************************************
/// Implementing a Model to view a list of CellHeaderData-records in a Model/TableView widget
/*!
***************************************************************************** */
class HeaderTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    HeaderTableModel(QObject * parent = 0);
    ~HeaderTableModel();

    void init(const std::vector<CellS57_Header> & catCnt);
    void init(QStringList cellFileList);
 
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole ) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

protected:

    std::vector <CellS57_Header> cells;
};




}
#endif