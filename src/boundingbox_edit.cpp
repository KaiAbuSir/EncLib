//*****************************************************************************
//** Copyright (C) 2010 Kai R. Neufeldt, Ahrensburg, Germany
//** This file is part of the ENClib
//** The ENC lib may be used unter the GPL General Public License Version 2
//** or with a Commercial License granted by Kai R. Neufeldt
//** contact  Kai R. Neufeldt, Manhagener Allee 65, 22926 Ahrensburg, Germany
//*****************************************************************************

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QDoubleValidator>

#include "boundingbox_edit.h"

using namespace Enc;

//******************************************************************************
/// Bounding Box Edit Widget
/*!
***************************************************************************** */

BBoxEditor::BBoxEditor(QWidget * parent, const DegBBox & bBox) : QFrame(parent)
{
    setFrameShape(Box);
    grdLyt = new QGridLayout(this);
    northEdt = new QLineEdit(this);
    southEdt = new QLineEdit(this);
    eastEdt = new QLineEdit(this);
    westEdt = new QLineEdit(this);
    northEdt->setValidator(new QDoubleValidator(-90,90,4,this));
    southEdt->setValidator(new QDoubleValidator(-90,90,4,this));
    eastEdt->setValidator(new QDoubleValidator(-180,180,4,this));
    westEdt->setValidator(new QDoubleValidator(-180,180,4,this));

    grdLyt->addWidget(new QLabel(tr("North:")), 0, 0, Qt::AlignRight);
    grdLyt->addWidget(new QLabel(tr("South:")), 4, 0, Qt::AlignRight);
    grdLyt->addWidget(new QLabel(tr("West:")), 1, 0, Qt::AlignLeft);
    grdLyt->addWidget(new QLabel(tr("East:")), 1, 2, Qt::AlignRight);
    grdLyt->addWidget(new QLabel("    "), 3, 0, Qt::AlignRight);  //dummy label to make widget look symmetric
    grdLyt->addWidget(northEdt, 0, 1, Qt::AlignJustify);
    grdLyt->addWidget(southEdt, 4, 1, Qt::AlignJustify);
    grdLyt->addWidget(westEdt, 2, 0, Qt::AlignJustify);
    grdLyt->addWidget(eastEdt, 2, 2, Qt::AlignJustify);
}

//******************************************************************************
/// Set the values, or clear them if newBBox is not valid
/*!
***************************************************************************** */
void BBoxEditor::setBBox(const DegBBox & newBBox)
{
    if (newBBox.isValid())
    {
        northEdt->setText(QString::number(newBBox.NLAT));
        southEdt->setText(QString::number(newBBox.SLAT));
        eastEdt->setText(QString::number(newBBox.ELON));
        westEdt->setText(QString::number(newBBox.WLON));
    }
    else clear();
}

//******************************************************************************
/// clear all edit widgets
/*!
***************************************************************************** */
void BBoxEditor::clear()
{
    northEdt->clear();
    southEdt->clear();
    eastEdt->clear();
    westEdt->clear();
}

//******************************************************************************
/// Get the Bounding box values
/*!
***************************************************************************** */
DegBBox BBoxEditor::getBBox() const
{
    DegBBox bbox;
    bool nlatOk, slatOk, wlonOk, elonOk;
    bbox.NLAT = northEdt->text().toDouble(&nlatOk);
    bbox.SLAT = southEdt->text().toDouble(&slatOk);
    bbox.ELON = eastEdt->text().toDouble(&elonOk);
    bbox.WLON = westEdt->text().toDouble(&wlonOk);
    if (!nlatOk || !slatOk || !wlonOk || !elonOk)
    {
        bbox = DegBBox();  //bbox invalid 
    }
    return bbox;
}

//******************************************************************************
//******************************************************************************
//******************************************************************************
/// Bounding Box Edit Dialog
/*!
***************************************************************************** */

BBoxEditDialog::BBoxEditDialog(QWidget * parent, const DegBBox & bBox) : QDialog(parent)
{
    QVBoxLayout * lyt = new QVBoxLayout(this);
    bbEdit = new BBoxEditor(this, bBox);
    lyt->addWidget(bbEdit);

    okBtn = new QPushButton("OK", bbEdit);
    bbEdit->grdLyt->addWidget(okBtn, 2, 1, Qt::AlignJustify);
    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
}

