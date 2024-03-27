#include "App.h"

#include "MainWindow.h"
#include "Modeling/CtDataCsgTree.h"
#include "Modeling/ImplicitCtStructure.h"
#include "Artifacts/GaussianArtifact.h"
#include "Artifacts/ImageArtifactConcatenation.h"
#include "Artifacts/ImageArtifactComposition.h"
#include "Artifacts/PipelineList.h"
#include "Artifacts/Pipeline.h"

#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>

App* App::Self = nullptr;

App* App::CreateInstance(int argc, char* argv[]) {
    if (Self) {
        qWarning("App already exists. Cannot create new instance.");
        return nullptr;
    }

    Self = new App(argc, argv);
    return Self;
}

App* App::GetInstance() {
    if (!Self) {
        qWarning("No instance exists. Instance needs to be created first.");
        return nullptr;
    }
    return Self;
}

App::App(int argc, char* argv[]) :
        Argc(argc),
        Argv(argv),
        QApp(*new QApplication(Argc, Argv)),
        CtDataTree(*CtDataCsgTree::New()),
        Pipelines(*PipelineList::New()),
        MainWin(nullptr) {
}

App::~App() {
    CtDataTree.Delete();
    Pipelines.Delete();
    delete &MainWin;

    QApplication::quit();
    delete &QApp;
}

int App::Run() {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
    smpToolsApi.SetBackend("STDTHREAD");
    qWarning(QString("Backend: %1").arg(smpToolsApi.GetBackend()).toStdString().c_str());

    InitializeWithTestData();

    MainWin = new MainWindow();
    MainWin->show();

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
    implicitCtStructure1->SetTransform({ 40.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    vtkNew<ImplicitCtStructure> implicitCtStructure2;
    implicitCtStructure2->SetImplicitFunction(ImplicitCtStructure::ImplicitFunctionType::BOX);
    implicitCtStructure2->SetTissueType(ImplicitCtStructure::GetTissueOrMaterialTypeByName("Cortical Bone"));
//
//    vtkNew<ImplicitCtStructure> implicitCtStructure3;
//    implicitCtStructure3->SetImplicitFunction(ImplicitCtStructure::ImplicitFunctionType::BOX);
//    implicitCtStructure3->SetTissueType(ImplicitCtStructure::GetTissueOrMaterialTypeByName("Soft Tissue"));

    CtDataTree.AddImplicitCtStructure(*implicitCtStructure1);
    CtDataTree.CombineWithImplicitCtStructure(*implicitCtStructure2, ImplicitStructureCombination::OperatorType::INTERSECTION);
//    CtDataTree->CombineWithImplicitCtStructure(*implicitCtStructure3, ImplicitStructureCombination::OperatorType::UNION);
//    CtDataTree->RemoveImplicitCtStructure(*implicitCtStructure2);
//    CtDataTree->RefineWithImplicitStructure({ "a", "", {}, {}, "Water", {}}, *implicitCtStructure3);

    vtkNew<Pipeline> pipeline;
    pipeline->SetCtDataTree(&CtDataTree);
    Pipelines.AddPipeline(pipeline);
    ImageArtifactConcatenation& imageArtifactConcatenation = pipeline->GetImageArtifactConcatenation();

    vtkNew<GaussianArtifact> gaussianArtifact;
    gaussianArtifact->SetName("sequential gaussian");
    imageArtifactConcatenation.AddImageArtifact(*gaussianArtifact);

    vtkNew<ImageArtifactComposition> imageArtifactComposition;
    imageArtifactComposition->SetCompType(ImageArtifactComposition::CompositionType::PARALLEL);
    vtkNew<ImageArtifactComposition> imageArtifactComposition1;
    imageArtifactComposition1->SetCompType(ImageArtifactComposition::CompositionType::SEQUENTIAL);
    vtkNew<GaussianArtifact> gaussianArtifact1;
    imageArtifactComposition->AddImageArtifact(*imageArtifactComposition1);
    imageArtifactComposition->AddImageArtifact(*gaussianArtifact1);
    imageArtifactConcatenation.AddImageArtifact(*imageArtifactComposition);

    vtkNew<GaussianArtifact> gaussianArtifact2;
    vtkNew<GaussianArtifact> gaussianArtifact3;
    imageArtifactComposition1->AddImageArtifact(*gaussianArtifact2);
    imageArtifactComposition1->AddImageArtifact(*gaussianArtifact3);

    vtkNew<GaussianArtifact> gaussianArtifact4;
    imageArtifactConcatenation.AddImageArtifact(*gaussianArtifact4);
}

CtDataCsgTree& App::GetCtDataCsgTree() const {
    return CtDataTree;
}

PipelineList& App::GetPipelines() const {
    return Pipelines;
}
