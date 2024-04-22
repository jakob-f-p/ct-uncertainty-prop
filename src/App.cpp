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

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>

#include "vtkNew.h"

App::App(int argc, char* argv[]) :
        Argc(argc),
        Argv(argv),
        QApp(new QApplication(Argc, Argv)),
        CtDataTree(new CtStructureTree()),
        Pipelines(new PipelineList(*CtDataTree)),
        MainWin(nullptr) {

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
//    smpToolsApi.SetBackend("STDTHREAD");
}

App::~App() {
    QApplication::quit();
}

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

auto App::Run() -> int {
    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
    qWarning(("Backend: " + std::string(smpToolsApi.GetBackend())).c_str());

    InitializeWithTestData();

    MainWin.reset(new MainWindow());
    MainWin->show();

    return QApplication::exec();
}

auto App::Quit() -> int {
    if (Self) {
        QApplication::quit();
        delete Self;
    }

    return 0;
}

auto App::GetCtDataTree() const -> CtStructureTree& {
    return *CtDataTree;
}

auto App::GetPipelines() const -> PipelineList& {
    return *Pipelines;
}

void App::InitializeWithTestData() {
    auto& pipeline = Pipelines->AddPipeline();

    BasicStructure sphere(FunctionType::SPHERE);
    sphere.SetTissueType(BasicStructureDetails::GetTissueTypeByName("Cancellous Bone"));
    sphere.SetTransformData({ 40.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    BasicStructure box(FunctionType::BOX);
    box.SetTissueType(BasicStructureDetails::GetTissueTypeByName("Cortical Bone"));
    box.SetTransformData({ 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    CombinedStructure combinedStructure;
    combinedStructure.SetOperatorType(OperatorType::UNION);
    combinedStructure.SetTransformData({ -10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    CtDataTree->AddBasicStructure(std::move(sphere));
    CtDataTree->CombineWithBasicStructure(std::move(box), std::move(combinedStructure));

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
