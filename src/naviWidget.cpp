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

    connect(naviView, SIGNAL(projectionChanged(int)), this, SLOT(projectionChanged(int)));
    connect(naviScene, SIGNAL(contentChanged()), naviView, SLOT(update()));
    connect(naviScene, SIGNAL(progressMessage(const QString &)), this, SIGNAL(progressMessage(const QString &)));
}

void NaviWidget::loadCharts(QStringList filenames)
{
    naviScene->loadCharts(filenames); //will redraw cells, too
}

void NaviWidget::projectionChanged(int prjktIt)
{
    naviScene->setProjection(prjktIt);
    naviScene->onDrawCells();    //kai - mal gucken , ob man view noch benachrichtigen muss
}
