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
    vtkNew<ImplicitCtStructure> implicitCtStructure1;
    implicitCtStructure1->SetImplicitFunction(ImplicitCtStructure::ImplicitFunctionType::SPHERE);
    implicitCtStructure1->SetTissueType(ImplicitCtStructure::GetTissueOrMaterialTypeByName("Cancellous Bone"));

    vtkNew<ImplicitCtStructure> implicitCtStructure2;
    implicitCtStructure2->SetImplicitFunction(ImplicitCtStructure::ImplicitFunctionType::BOX);
    implicitCtStructure2->SetTissueType(ImplicitCtStructure::GetTissueOrMaterialTypeByName("Cortical Bone"));

    vtkNew<ImplicitCtStructure> implicitCtStructure3;
    implicitCtStructure3->SetImplicitFunction(ImplicitCtStructure::ImplicitFunctionType::BOX);
    implicitCtStructure3->SetTissueType(ImplicitCtStructure::GetTissueOrMaterialTypeByName("Soft Tissue"));

    CtDataTree->AddImplicitCtStructure(*implicitCtStructure1);
    CtDataTree->CombineWithImplicitCtStructure(*implicitCtStructure2, ImplicitStructureCombination::OperatorType::UNION);
    CtDataTree->CombineWithImplicitCtStructure(*implicitCtStructure3, ImplicitStructureCombination::OperatorType::INTERSECTION);

    CtDataTree->RemoveImplicitCtStructure(*implicitCtStructure2);
    CtDataTree->Print(std::cout);
}

CtDataCsgTree* App::GetCtDataCsgTree() const {
    return CtDataTree;
}
