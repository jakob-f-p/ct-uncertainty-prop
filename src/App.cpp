#include "App.h"
#include "MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

int App::run(int argc, char *argv[]) {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
}