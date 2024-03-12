#include "App.h"
#include "MainWindow.h"

#include "./Modeling/CtDataCsgTree.h"

#include <vtkSphere.h>

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>


int App::run(int argc, char *argv[]) {
    vtkNew<CtDataCsgTree> tree;

    vtkNew<vtkSphere> implicitSphere;
    implicitSphere->SetRadius(10.0);
    vtkNew<ImplicitCtStructure> implicitCtStructure1;
    implicitCtStructure1->SetImplicitFunction(implicitSphere);
    implicitCtStructure1->SetTissueType(CT::GetTissueOrMaterialTypeByName("Air"));

    tree->AddImplicitCtStructure(*implicitCtStructure1);
    tree->Print(std::cout);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
}