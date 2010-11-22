#include "naviWidget.h"

#include "naviScene.h"
#include "naviView.h"

#include <QHBoxLayout>

using namespace Enc;

NaviWidget::NaviWidget(QWidget * parent) : QWidget(parent)
{
    naviScene = new NaviScene(this);
    naviView = new NaviView(naviScene);

    QHBoxLayout * lyt = new QHBoxLayout(this);
    lyt->addWidget(naviView);

    connect(naviView, SIGNAL(projectionChange(int)), this, SLOT(projectionChange(int)));
    connect(naviScene, SIGNAL(contentChanged()), naviView, SLOT(update()));
}

void NaviWidget::loadCharts(QStringList filenames)
{
    naviScene->loadCharts(filenames);
}

void NaviWidget::projectionChange(int prjktIt)
{
    naviScene->setProjection(prjktIt);
    naviScene->onDrawCells();    //kai - mal gucken , ob man view noch benachrichtigen muss
}
