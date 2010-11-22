#ifndef NAVIWIDGET_H
#define NAVIWIDGET_H

#include <QWidget>

namespace Enc
{
    class NaviScene;
    class NaviView;

//*****************************************************************************
/// High Level, Easy2Use Widget to display a Chart and appropriate Navi-Buttons
/*!
  *
  *************************************************************************** */
class NaviWidget : public QWidget
{
    Q_OBJECT

public:
    NaviWidget(QWidget * parent = 0);

public slots:

    void loadCharts(QStringList filenames);
    void projectionChange(int);

protected:

    NaviScene * naviScene;
    NaviView * naviView;
};



}

#endif // NAVIWIDGET_H
