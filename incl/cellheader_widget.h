#ifndef CELL_HEADER_WIDGET_H
#define CELL_HEADER_WIDGET_H

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QTableWidget>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtCore/QString>
#include <QtCore/QAbstractTableModel>
#include <QtGui/QSortFilterProxyModel>

#include "cellheader_tabmodel.h"

namespace Enc
{
class CellHeaderReader;
class HeaderTableView;
struct CellHeaderData;

//******************************************************************************
/// Table View, reimplementet to allow hiding of Columns comfortably
/*!
***************************************************************************** */
class HeaderTableView : public QTableView
{
    Q_OBJECT
public:

    HeaderTableView(QWidget *parent);
    virtual ~HeaderTableView();

public slots:

    void showAllColumns(bool showAll);

protected:

};

//******************************************************************************
/// Simple Widget to display Cell Header Data of several Cells in a Table
/*!
***************************************************************************** */
class CellHeaderDialog : public QDialog
{
    Q_OBJECT
public:
    CellHeaderDialog(QWidget * parent =0);
    virtual ~CellHeaderDialog();

private slots:
    void onOpenFiles();
    void onOpenDir();
    //void onCancel();
    //void onFinished();

private:

    void paresCells(QStringList files);
 
    QLineEdit * fileNameEdt;
    HeaderTableView * headerDataTbl;
    QProgressBar * progBar;
    QPushButton * cancelBtn;

    HeaderTableModel * headerTableModel;

    double progVal, progStep;
};

//******************************************************************************
/// Proxy class to Sort and Filter the HeaderTableModel
/*!
***************************************************************************** */
class HeaderTableSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    HeaderTableSortFilterModel(QObject * parent = 0);

public slots:


protected:

    virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;
    virtual bool lessThan(const QModelIndex & left, const QModelIndex & right)const;
};

}

#endif