#ifndef EXSET_FILTER_WIDGET_H
#define EXSET_FILTER_WIDGET_H


#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QLineEdit>
#include <QtGui/QSortFilterProxyModel>

#include <vector>

#include "boundingbox_degrees.h"

namespace Enc
{


//******************************************************************************
/// Structur stores all Exchange Set Filter Settings
/*!
***************************************************************************** */
struct ExSetFilterSettings
{
    enum GeneralAction{DoFilter =1, DoFind =2};
    GeneralAction genAct;

    enum FileNameFilterType{NoFileFilter =0, FixedString=1, WildCard=2, RexExp=3};
    FileNameFilterType fileNameFilter;
    QString regExp;


    bool doFilterBBox;
    DegBBox bBox2filter;
    bool inside; //all cells inside the filterBox pass the filter

    ExSetFilterSettings() : genAct(DoFilter), fileNameFilter(NoFileFilter), doFilterBBox(false), inside(true)
    {}
    bool filteringPossible(QString & usgMessage) const;
    bool operator==(const ExSetFilterSettings & filter2) const;
    bool operator!=(const ExSetFilterSettings & filter2) const {return !(*this == filter2);}
};

//******************************************************************************
/// Widget to display/set ExSetFilterSettings
/*!
***************************************************************************** */
class ExSetFilterWidget : public QWidget
{
    Q_OBJECT

signals:
    void showAllColumns(bool showAll);
    void filter(ExSetFilterSettings);
    void unFilter(); //show all rows, again

public:
    ExSetFilterWidget(QWidget * parent);
    ~ExSetFilterWidget();
    bool allColumnsOn() const {return showAllBtn->isDown();}
    ExSetFilterSettings getFilterSettings() const;

private slots:

    void onFilterAll(bool);
    void onFindNext();

    void onEnableFileFilter(bool);
    void onEnableBBoxFilter(bool);

    void onEditBBox();

private:

    QCheckBox * useFNameCBx;
    QCheckBox * useBBoxCBx;
    QLineEdit * searchNameEdt;
    QLineEdit * searchBBoxEdt;

    QPushButton * editBBoxBtn;

    QPushButton * findNextBtn;
    QPushButton * filterAllBtn;
    QPushButton * showAllBtn;
};

}
#endif