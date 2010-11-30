#ifndef NAVIWIDGET_H
#define NAVIWIDGET_H

#include <QtGui/QWidget>

#include "naviScene.h"
#include "naviView.h"

namespace Enc
{

//*****************************************************************************
/// High Level, Easy2Use Widget to display a Chart and appropriate Navi-Buttons
/*!
  *
  *************************************************************************** */
class NaviWidget : public QWidget
{
    Q_OBJECT

signals:
    void progressMessage(const QString &);
    void projectionChanged(int);
    void scaleChanged(double);
    void headingChanged(double);

public:
    NaviWidget(QWidget * parent = 0);

public slots:

    void loadCharts(QStringList filenames);
    void onProjectionChanged(int);
    void zoomIn() {naviView->zoomIn();}
    void zoomOut() {naviView->zoomOut();}
    void setScale(double newScale) {naviView->setScale(newScale);}
    void setChartHeading(double heading) {naviView->setChartHeading(heading);}

protected:

    NaviScene * naviScene;
    NaviView * naviView;
};



}

#endif // NAVIWIDGET_H
