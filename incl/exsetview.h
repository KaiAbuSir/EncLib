#ifndef EXSETVIEW_H
#define EXSETVIEW_H

#include <QtGui/QTableView>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QLineEdit>
#include <QtGui/QSortFilterProxyModel>

#include <vector>

#include "boundingbox_degrees.h"
#include "exSetFilterWidget.h"  //moc wants full type, not forward declaration

namespace Enc
{
struct CatalogEntry;

//******************************************************************************
/// Widget displaying the contents of a CATALOG.031 File
/*!
***************************************************************************** */
class ExSetView : public QTableView
{
    Q_OBJECT
public:

    ExSetView(QWidget *parent);
    virtual ~ExSetView();

public slots:

    void showAllColumns(bool showAll);
    void doFilter(ExSetFilterSettings);
    void unFilter();

protected:

    ExSetFilterSettings currentFilterSettings;
  
};


//******************************************************************************
///
/*!
***************************************************************************** */
class ExSetCheckWidget : public QWidget
{
    Q_OBJECT

signals:
    void checkAll();
    void checkSelected();

public:

    ExSetCheckWidget(QWidget * parent =0);

protected:
    QPushButton * checkAllBtn;
    QPushButton * checkSelBtn;
};

//******************************************************************************
/// Implementing a model to used a Catalog.031 in a Model/View widget
/*!
***************************************************************************** */
class ExSetModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ExSetModel(QObject * parent = 0);
    ~ExSetModel();

    void init(const std::vector<CatalogEntry> & catCnt);
    void init(QString catFileName);

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex() ) const;
    //virtual QModelIndex parent(const QModelIndex & index) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole ) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

protected:

    std::vector<CatalogEntry> catCont;
};

//******************************************************************************
///
/*!
***************************************************************************** */

class ExSetSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    ExSetSortFilterModel(QObject * parent = 0);

public slots:

    void filterFileName(QString regExp);

protected:

    virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;
    virtual bool lessThan(const QModelIndex & left, const QModelIndex & right)const;
};

}
#endif // EXSETVIEW_H
