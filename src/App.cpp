#include "App.h"

#include "MainWindow.h"
#include "Modeling/BasicStructure.h"
#include "Modeling/CombinedStructure.h"
#include "Modeling/CtDataSource.h"
#include "Modeling/CtStructureTree.h"
#include "Artifacts/ImageArtifact.h"
#include "Artifacts/ImageArtifactConcatenation.h"
#include "Artifacts/CompositeImageArtifact.h"
#include "Artifacts/StructureArtifact.h"
#include "Artifacts/StructureArtifactListCollection.h"
#include "Artifacts/PipelineList.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <memory>
#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>

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

    MainWin = std::make_unique<MainWindow>();
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

    StructureArtifact motionArtifact1(StructureArtifact::SubType::MOTION);
    motionArtifact1.SetName("1");

    StructureArtifact motionArtifact2(StructureArtifact::SubType::MOTION);
    motionArtifact2.SetName("2");

    auto& basicStructure1Artifacts = pipeline.GetStructureArtifactListCollectionForIdx(1);
    basicStructure1Artifacts.AddStructureArtifact(motionArtifact1);
    basicStructure1Artifacts.AddStructureArtifact(motionArtifact2);


    ImageArtifactConcatenation& imageArtifactConcatenation = pipeline.GetImageArtifactConcatenation();

    BasicImageArtifact gaussianArtifact(BasicImageArtifactDetails::SubType::GAUSSIAN);
    gaussianArtifact.SetName("sequential gaussian");
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact));

    CompositeImageArtifact compositeImageArtifact;
    compositeImageArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    auto& composite = imageArtifactConcatenation.AddImageArtifact(std::move(compositeImageArtifact));

    CompositeImageArtifact compositeImageArtifact1;
    compositeImageArtifact1.SetCompositionType(CompositeImageArtifactDetails::CompositionType::SEQUENTIAL);
    auto& composite1 = imageArtifactConcatenation.AddImageArtifact(std::move(compositeImageArtifact1), &composite);
    BasicImageArtifact gaussianArtifact2(BasicImageArtifactDetails::SubType::GAUSSIAN);
    BasicImageArtifact gaussianArtifact3(BasicImageArtifactDetails::SubType::GAUSSIAN);
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact2), &composite1);
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact3), &composite1);

    BasicImageArtifact gaussianArtifact1(BasicImageArtifactDetails::SubType::GAUSSIAN);
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact1), &composite);

    BasicImageArtifact gaussianArtifact4(BasicImageArtifactDetails::SubType::GAUSSIAN);
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact4));
}
