#include "App.h"

#include "MainWindow.h"
#include "Modeling/BasicStructure.h"
#include "Modeling/CombinedStructure.h"
#include "Modeling/CtStructure.h"
#include "Modeling/CtStructureTree.h"
#include "Artifacts/GaussianArtifact.h"
#include "Artifacts/ImageArtifactConcatenation.h"
#include "Artifacts/CompositeArtifact.h"
#include "Artifacts/MotionArtifact.h"
#include "Artifacts/PipelineList.h"

#include "vtkNew.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>

App* App::Self = nullptr;

auto App::CreateInstance(int argc, char* argv[]) -> App* {
    if (Self)
        throw std::runtime_error("App already exists. Cannot create new instance.");

    Self = new App(argc, argv);
    return Self;
}

auto App::GetInstance() -> App* {
    if (!Self)
        throw std::runtime_error("No instance exists. Instance needs to be created first.");

    return Self;
}

App::App(int argc, char* argv[]) :
        Argc(argc),
        Argv(argv),
        QApp(new QApplication(Argc, Argv)),
        CtDataTree(new CtStructureTree()),
        Pipelines(new PipelineList(*CtDataTree)),
        MainWin(nullptr) {
}

App::~App() {
    delete &MainWin;

    QApplication::quit();
    delete QApp;
}

auto App::Run() -> int {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
    smpToolsApi.SetBackend("STDTHREAD");
    qWarning(("Backend: " + std::string(smpToolsApi.GetBackend())).c_str());

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
    auto& pipeline = Pipelines->AddPipeline();

    SphereStructure sphereStructure;
    sphereStructure.SetTissueType(CtStructureBase::GetTissueTypeByName("Cancellous Bone"));
    sphereStructure.SetTransformData({ 40.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    BoxStructure boxStructure;
    boxStructure.SetTissueType(CtStructureBase::GetTissueTypeByName("Cortical Bone"));

    CombinedStructure combinedStructure;
    combinedStructure.SetOperatorType(CtStructureBase::OperatorType::INTERSECTION);

    CtDataTree->AddBasicStructure(std::move(sphereStructure));
    CtDataTree->CombineWithBasicStructure(std::move(boxStructure), std::move(combinedStructure));

    vtkNew<MotionArtifact> motionArtifact1;
    motionArtifact1->SetName("1");

    vtkNew<MotionArtifact> motionArtifact2;
    motionArtifact2->SetName("2");

    auto& basicStructure1Artifacts = pipeline.GetArtifactStructureWrapper(1);
    basicStructure1Artifacts.AddStructureArtifact(*motionArtifact1);
    basicStructure1Artifacts.AddStructureArtifact(*motionArtifact2);


    ImageArtifactConcatenation& imageArtifactConcatenation = pipeline.GetImageArtifactConcatenation();

    vtkNew<GaussianArtifact> gaussianArtifact;
    gaussianArtifact->SetName("sequential gaussian");
    imageArtifactConcatenation.AddImageArtifact(*gaussianArtifact);

    vtkNew<CompositeArtifact> imageArtifactComposition;
    imageArtifactComposition->SetCompType(CompositeArtifact::CompositionType::PARALLEL);
    vtkNew<CompositeArtifact> imageArtifactComposition1;
    imageArtifactComposition1->SetCompType(CompositeArtifact::CompositionType::SEQUENTIAL);
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

auto App::GetCtDataTree() const -> CtStructureTree& {
    return *CtDataTree;
}

auto App::GetPipelines() const -> PipelineList& {
    return *Pipelines;
}
