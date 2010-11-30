#include "naviWidget.h"

#include "naviScene.h"
#include "naviView.h"

#include <QtGui/QHBoxLayout>

using namespace Enc;

NaviWidget::NaviWidget(QWidget * parent) : QWidget(parent)
{
    naviScene = new NaviScene(this);
    naviView = new NaviView(naviScene);

    QHBoxLayout * lyt = new QHBoxLayout(this);
    lyt->addWidget(naviView);

    connect(naviView, SIGNAL(projectionChanged(int)), this, SLOT(onProjectionChanged(int)));

    connect(naviView, SIGNAL(scaleChanged(double)), this, SIGNAL(scaleChanged(double)));
    connect(naviView, SIGNAL(headingChanged(double)), this, SIGNAL(headingChanged(double)));

    connect(naviScene, SIGNAL(contentChanged(QRectF)), naviView, SLOT(showContent(QRectF)));
    connect(naviScene, SIGNAL(progressMessage(const QString &)), this, SIGNAL(progressMessage(const QString &)));
}

void NaviWidget::loadCharts(QStringList filenames)
{
    naviScene->loadCharts(filenames); //will redraw cells, too
}

void NaviWidget::onProjectionChanged(int prjktIt)
{
    naviScene->setProjection(prjktIt);
    naviScene->onDrawCells();    //kai - mal gucken , ob man view noch benachrichtigen muss
}
