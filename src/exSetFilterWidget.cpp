//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>

#include "catalog031reader.h"
#include "exSetFilterWidget.h"
#include "boundingbox_edit.h"

using namespace Enc;

//******************************************************************************
/// Check if filter Settings are logical equal
/*!
* if only the not-enabled settings are different, Settings are still treated as equal!
***************************************************************************** */
bool ExSetFilterSettings::operator==(const ExSetFilterSettings & filter2) const
{
    const ExSetFilterSettings & filter1 = *this;
    if (filter1.genAct != filter2.genAct) return false;

    enum FileNameFilterType{NoFileFilter =0, FixedString=1, WildCard=2, RexExp=3};
    if (filter1.fileNameFilter != filter2.fileNameFilter) return false;
    else if (filter1.fileNameFilter != NoFileFilter)
    {
        if (filter1.regExp != filter2.regExp) return false;
    }

    if (filter1.doFilterBBox != filter2.doFilterBBox) return false;
    else if (filter1.doFilterBBox)
    {
        if (filter1.bBox2filter != filter2.bBox2filter) return false;
    }
    if (filter1.inside != filter2.inside) return false;

    return true;
}

//******************************************************************************
/// Check if filter settings are usable for filtering
//**************************************************************************** */
bool ExSetFilterSettings::filteringPossible(QString & usrMsg) const
{
    if (fileNameFilter == NoFileFilter && !doFilterBBox) 
    {
        usrMsg += QObject::tr("ERROR: Neither Filename Filter nor Bounding Box Filter enabled!");
        return false;
    }
    if (fileNameFilter != NoFileFilter)
    {
        if (regExp.isEmpty())
        {
            usrMsg += QObject::tr("ERROR: Empty Filter Expression for Filename!");
            return false;
        }
        //check regExp syntay ?????
    }
    if (doFilterBBox && !bBox2filter.isValid())
    {
        usrMsg += QObject::tr("ERROR: Bounding Box Filter enabled, but Bounding Box not valiD!");
        return false;
    }
    return true;
}

//******************************************************************************
/// Construct a Widget where the user can select Filter Settings
//******************************************************************************
ExSetFilterWidget::ExSetFilterWidget(QWidget * parent) : QWidget(parent)
{
    QHBoxLayout * mainLyt = new QHBoxLayout(this);
    
    QFrame * settingsFrm = new QFrame(this);
    settingsFrm->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mainLyt->addWidget(settingsFrm, 1);
    QVBoxLayout * filterItemsLyt = new QVBoxLayout(settingsFrm);
    QHBoxLayout * nameFilterLyt = new QHBoxLayout();
    QHBoxLayout * bboxFilterLyt = new QHBoxLayout(); 
    filterItemsLyt->addLayout(nameFilterLyt, 0);
    filterItemsLyt->addLayout(bboxFilterLyt, 0);

    //**** Checkboxes to enable items ****
    useFNameCBx = new QCheckBox(settingsFrm);
    useBBoxCBx =  new QCheckBox(settingsFrm);

    //**** name Filter widgets ****
    QLabel * searchNameLbl = new QLabel(tr("Filename: "), settingsFrm);
    searchNameEdt = new QLineEdit(settingsFrm);
    nameFilterLyt->addWidget(useFNameCBx, 0);
    nameFilterLyt->addWidget(searchNameLbl, 0);
    nameFilterLyt->addWidget(searchNameEdt, 1);
  
    //**** bounding Box widgets ****
    QLabel * bBoxLbl = new QLabel(tr("Bounding Box: "), settingsFrm);
    searchBBoxEdt = new QLineEdit(settingsFrm);
    editBBoxBtn = new QPushButton("Edit", settingsFrm);
    editBBoxBtn->setFixedSize(editBBoxBtn->sizeHint());
    bboxFilterLyt->addWidget(useBBoxCBx, 0);
    bboxFilterLyt->addWidget(bBoxLbl, 0);
    bboxFilterLyt->addWidget(searchBBoxEdt, 1);
    bboxFilterLyt->addWidget(editBBoxBtn, 0);

    //**** the "Start Filter" buttons ****
    QFrame * buttonsFrm = new QFrame(this);
    buttonsFrm->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mainLyt->addWidget(buttonsFrm, 0);
    QVBoxLayout * btnLyt = new QVBoxLayout(buttonsFrm);
    findNextBtn = new QPushButton(tr("Find next"), buttonsFrm);
    filterAllBtn = new QPushButton(tr("Filter"), buttonsFrm);
    filterAllBtn->setCheckable(true);
    filterAllBtn->setChecked(false);

    btnLyt->addWidget(findNextBtn, 0);
    btnLyt->addWidget(filterAllBtn, 0);

    showAllBtn = new QPushButton(tr("Show\nAll"), this);
    showAllBtn->setCheckable(true);
    showAllBtn->setChecked(false);
    mainLyt->addWidget(showAllBtn, 0);

    connect(showAllBtn, SIGNAL(toggled(bool)), this, SIGNAL(showAllColumns(bool)));
    connect(filterAllBtn, SIGNAL(toggled(bool)), this, SLOT(onFilterAll(bool)));
    connect(findNextBtn, SIGNAL(clicked()), this, SLOT(onFindNext()));

    connect(useFNameCBx, SIGNAL(toggled(bool)), this, SLOT(onEnableFileFilter(bool)));
    connect(useBBoxCBx, SIGNAL(toggled(bool)), this, SLOT(onEnableBBoxFilter(bool)));

    connect(editBBoxBtn, SIGNAL(clicked()), this, SLOT(onEditBBox()));

    //** inits **
    onEnableFileFilter(useFNameCBx->isDown());
    onEnableBBoxFilter(useBBoxCBx->isDown());

    //**** tooltips ****
    useFNameCBx->setToolTip(tr("Enable the File Name Filter"));
    useBBoxCBx->setToolTip(tr("Enable the Bounding Box Filter"));

    findNextBtn->setToolTip(tr("Find and Select next matching Record"));
    filterAllBtn->setToolTip(tr("Show only those Records that pass the Filter"));
    showAllBtn->setToolTip(tr("Show even unimportant Columns, which have no menaning for the Exchange Set"));
}

ExSetFilterWidget::~ExSetFilterWidget()
{}

//******************************************************************************
/// Slot Called if user wants to find next matching item
/*!
***************************************************************************** */
void ExSetFilterWidget::onFindNext()
{
    //**** Filtering AND finding makes no sense! ****
    if (filterAllBtn->isDown())
    {
        filterAllBtn->setDown(false);
        emit unFilter();
    }

    //**** get the Filter Settings, check them, and send filter-signal ****
    ExSetFilterSettings filterSettings = getFilterSettings();
    filterSettings.genAct = ExSetFilterSettings::DoFind;

    QString usrMsg;
    bool filterOk = filterSettings.filteringPossible(usrMsg);
    if (!filterOk)
    {
        QMessageBox::warning(this, "ExSetCheck", usrMsg);
        return;
    }
    emit filter(filterSettings);
}

//******************************************************************************
/// Slot called if user wants to filter whole Exchange Set at once (or stop Filtering)
/*!
***************************************************************************** */
void ExSetFilterWidget::onFilterAll(bool filterOn)
{
    if (filterOn)
    {
        ExSetFilterSettings filterSettings = getFilterSettings();
        filterSettings.genAct = ExSetFilterSettings::DoFilter;
        QString usrMsg;
        bool filterOk = filterSettings.filteringPossible(usrMsg);
        if (!filterOk)
        {
            QMessageBox::warning(this, "ExSetCheck", usrMsg);
            return;
        }
        
        emit filter(filterSettings);
    }
    else emit unFilter();
}

//******************************************************************************
/// Parse all Widgets to get the selected Filter Settings
//******************************************************************************
ExSetFilterSettings ExSetFilterWidget:: getFilterSettings() const
{
    ExSetFilterSettings filterSettings;

    if (useFNameCBx->isDown()) filterSettings.fileNameFilter = ExSetFilterSettings::WildCard;
    filterSettings.regExp = searchNameEdt->text().simplified();

    filterSettings.doFilterBBox = useBBoxCBx->isDown();
    filterSettings.bBox2filter.fromString(searchBBoxEdt->text().simplified());

    return filterSettings;
}

//******************************************************************************
/// Enable FileName Widgets only if Filename-Filtering is enabled
//******************************************************************************
void ExSetFilterWidget::onEnableFileFilter(bool on)
{
    searchNameEdt->setEnabled(on);
}

//******************************************************************************
/// Enable BoundingBox Widgets only if BoundingBox is enabled
//******************************************************************************
void ExSetFilterWidget::onEnableBBoxFilter(bool on)
{
    editBBoxBtn->setEnabled(on);
    searchBBoxEdt->setEnabled(on);
}

//******************************************************************************
/// Enable BoundingBox Widgets only if BoundingBox is enabled
//******************************************************************************
void ExSetFilterWidget::onEditBBox()
{
    DegBBox bBox(searchBBoxEdt->text());
    BBoxEditDialog * bbDlg = new BBoxEditDialog(this, bBox);
    if (bbDlg->exec() == QDialog::Accepted)
    {
        searchBBoxEdt->setText(bbDlg->getBBox().toString());
    }
}