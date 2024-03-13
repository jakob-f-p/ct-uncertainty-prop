#include "App.h"
#include "MainWindow.h"

#include <vtkSphere.h>

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

App* App::Self = nullptr;

App* App::CreateInstance(int argc, char* argv[]) {
    if (Self) {
        std::cout << "App already exists. Cannot create new instance." << std::endl;
        return nullptr;
    }

    Self = new App(argc, argv);
    return Self;
}

App* App::GetInstance() {
    if (!Self) {
        std::cout << "No instance exists. Instance needs to be created first." << std::endl;
        return nullptr;
    }
    return Self;
}

App::App(int argc, char* argv[]) : Argc(argc), Argv(argv) {
    QApp = new QApplication(Argc, Argv);
    CtDataTree = CtDataCsgTree::New();
}

App::~App() {
    CtDataTree->Delete();

    if (QApp) {
        QApplication::quit();
        delete QApp;
    }
}

int App::Run() {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    InitializeWithTestData();

    MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
}

int App::Quit() {
    if (Self) {
        QApplication::quit();
        delete Self;
    }

    return 0;
}

void App::InitializeWithTestData() {
    vtkNew<vtkSphere> implicitSphere;
    implicitSphere->SetRadius(10.0);
    vtkNew<ImplicitCtStructure> implicitCtStructure1;
    implicitCtStructure1->SetImplicitFunction(implicitSphere);
    implicitCtStructure1->SetTissueType(CT::GetTissueOrMaterialTypeByName("CancellousBone"));

    vtkNew<ImplicitCtStructure> implicitCtStructure2;
    implicitCtStructure2->SetImplicitFunction(implicitSphere);
    implicitCtStructure2->SetTissueType(CT::GetTissueOrMaterialTypeByName("CorticalBone"));

    vtkNew<ImplicitCtStructure> implicitCtStructure3;
    implicitCtStructure3->SetImplicitFunction(implicitSphere);
    implicitCtStructure3->SetTissueType(CT::GetTissueOrMaterialTypeByName("SoftTissue"));

    CtDataTree->AddImplicitCtStructure(*implicitCtStructure1);
    CtDataTree->CombineWithImplicitCtStructure(*implicitCtStructure2, ImplicitStructureCombination::OperatorType::UNION);
    CtDataTree->CombineWithImplicitCtStructure(*implicitCtStructure3, ImplicitStructureCombination::OperatorType::UNION);

    CtDataTree->RemoveImplicitCtStructure(*implicitCtStructure2);
    CtDataTree->Print(std::cout);
}

const CtDataCsgTree* App::GetCtDataCsgTree() const {
    return CtDataTree;
}
