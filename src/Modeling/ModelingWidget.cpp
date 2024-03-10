#include <QDockWidget.h>
#include <QVTKOpenGLNativeWidget.h>

#include "ModelingWidget.h"

ModelingWidget::ModelingWidget() {
    auto* implicitModelRenderingWindow = new QVTKOpenGLNativeWidget();

    setCentralWidget(implicitModelRenderingWindow);


    auto* implicitDataTreeWidget = new QDockWidget();
    implicitDataTreeWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    implicitDataTreeWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea,
                  implicitDataTreeWidget,
                  Qt::Orientation::Vertical);
}
