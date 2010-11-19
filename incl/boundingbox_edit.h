#ifndef BOUNDING_BOX_EDITOR_H
#define BOUNDING_BOX_EDITOR_H

#include <QtCore/QString>
#include <QtGui/QWidget>
#include <QtGui/QFrame>
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>


#include "boundingbox_degrees.h"

namespace Enc
{
class BBoxEditDialog;

//******************************************************************************
/// Bounding Box Edit Widget
/*!
***************************************************************************** */
class BBoxEditor : public QFrame
{
friend class BBoxEditDialog;
public:
    BBoxEditor(QWidget * parent =0, const DegBBox & bBox = DegBBox());
    DegBBox getBBox() const;
    void setBBox(const DegBBox & newBBox);
    void BBoxEditor::clear();

protected:

   QLineEdit * northEdt;
   QLineEdit * southEdt;
   QLineEdit * eastEdt;
   QLineEdit * westEdt;
   QGridLayout * grdLyt;
};

//******************************************************************************
/// Bounding Box Edit Dialog
/*!
***************************************************************************** */
class BBoxEditDialog : public QDialog
{
public:
    BBoxEditDialog(QWidget * parent, const DegBBox & bBox);
    DegBBox getBBox() const {return bbEdit->getBBox();}
    void setBBox(const DegBBox & newBBox) {bbEdit->setBBox(newBBox);}
    void clear() {bbEdit->clear();}

protected:

    BBoxEditor * bbEdit;
    QPushButton * okBtn;
};

}
#endif

