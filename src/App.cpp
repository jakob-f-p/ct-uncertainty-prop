#include "App.h"

#include "Ui/MainWindow.h"
#include "Modeling/BasicStructure.h"
#include "Modeling/CombinedStructure.h"
#include "Modeling/CtDataSource.h"
#include "Modeling/CtStructureTree.h"
#include "Artifacts/Image/ImageArtifact.h"
#include "Artifacts/Image/ImageArtifactConcatenation.h"
#include "Artifacts/Image/CompositeImageArtifact.h"
#include "Artifacts/Structure/StructureArtifact.h"
#include "Artifacts/Structure/StructureArtifactListCollection.h"
#include "Artifacts/PipelineList.h"
#include "Segmentation/ThresholdFilter.h"
#include "PipelineGroups/PipelineGroupList.h"
#include "PipelineGroups/PipelineParameterSpan.h"
#include "Utils/PythonInterpreter.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>


App::App(int argc, char* argv[]) :
        Argc(argc),
        Argv(argv),
        QApp(new QApplication(Argc, Argv)),
        MainWin(nullptr),
        CtDataTree(new CtStructureTree()),
        DataSource([&]() {
            vtkNew<CtDataSource> dataSource;
            dataSource->SetDataTree(CtDataTree.get());
            return dataSource;
        }()),
        Pipelines(new PipelineList(*CtDataTree, *DataSource)),
        PipelineGroups([&pipelines = *Pipelines]() {
            auto* const pipelineGroupList = new PipelineGroupList(pipelines);

            auto const& removeDependentPipelineGroups = [pipelineGroupList](PipelineEvent const& event) {
                if (event.Type != PipelineEventType::PRE_REMOVE)
                    return;

                if (event.PipelinePointer == nullptr)
                    throw std::runtime_error("pipeline pointer may not be nullptr");

                auto pipelineGroups = pipelineGroupList->FindPipelineGroupsByBasePipeline(*event.PipelinePointer);
                for (auto const* group : pipelineGroups)
                    pipelineGroupList->RemovePipelineGroup(*group);
            };
            pipelines.AddPipelineEventCallback(removeDependentPipelineGroups);

            return pipelineGroupList;
        }()),
        PyInterpreter(new PythonInterpreter()) {

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

    MainWin = std::make_unique<MainWindow>(*CtDataTree, *DataSource, *ThresholdFilterAlgorithm,
                                           *Pipelines, *PipelineGroups);
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

auto App::GetThresholdFilter() const -> vtkImageAlgorithm& {
    return *ThresholdFilterAlgorithm;
}

auto App::GetPythonInterpreter() const -> PythonInterpreter& {
    return *PyInterpreter;
}

void App::InitializeWithTestData() {
    auto& pipeline = Pipelines->AddPipeline();

    BasicStructure sphere(Sphere{});
    sphere.SetTissueType(BasicStructureDetails::GetTissueTypeByName("Cancellous Bone"));
    sphere.SetTransformData({ 40.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    BasicStructure box(Box{});
    box.SetTissueType(BasicStructureDetails::GetTissueTypeByName("Cortical Bone"));
    box.SetTransformData({ 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    CombinedStructure combinedStructure;
    combinedStructure.SetOperatorType(CombinedStructure::OperatorType::UNION);
    combinedStructure.SetTransformData({ -10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    CtDataTree->AddBasicStructure(std::move(sphere));
    CtDataTree->CombineWithBasicStructure(std::move(box), std::move(combinedStructure));

    MotionArtifact motionArtifact {};
    motionArtifact.SetCtNumberFactor(5.0);
    motionArtifact.SetTransform({ 1.0, 0.0, 0.0,
                                  0.0, 0.0, 0.0,
                                  1.0, 1.0, 1.0 });
    StructureArtifact motionStructureArtifact(std::move(motionArtifact));
    motionStructureArtifact.SetName("1");

    auto& basicStructure1Artifacts = pipeline.GetStructureArtifactList(1);
    basicStructure1Artifacts.AddStructureArtifact(std::move(motionStructureArtifact));


    ImageArtifactConcatenation& imageArtifactConcatenation = pipeline.GetImageArtifactConcatenation();

    BasicImageArtifact gaussianArtifact(GaussianArtifact{});
    gaussianArtifact.SetName("sequential gaussian");
    auto& gaussian = imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact));

    CompositeImageArtifact compositeImageArtifact;
    compositeImageArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    auto& composite = imageArtifactConcatenation.AddImageArtifact(std::move(compositeImageArtifact));

    CompositeImageArtifact compositeImageArtifact1;
    compositeImageArtifact1.SetCompositionType(CompositeImageArtifactDetails::CompositionType::SEQUENTIAL);
    auto& composite1 = imageArtifactConcatenation.AddImageArtifact(std::move(compositeImageArtifact1), &composite);
    BasicImageArtifact gaussianArtifact2(GaussianArtifact{});
    BasicImageArtifact gaussianArtifact3(GaussianArtifact{});
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact2), &composite1);
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact3), &composite1);

    BasicImageArtifact gaussianArtifact1(GaussianArtifact{});
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact1), &composite);

    BasicImageArtifact gaussianArtifact4(GaussianArtifact{});
    imageArtifactConcatenation.AddImageArtifact(std::move(gaussianArtifact4));

    SaltPepperArtifact saltPepperArtifact;
    saltPepperArtifact.SetSaltAmount(0.001);
    saltPepperArtifact.SetSaltIntensity(1000.0);
    imageArtifactConcatenation.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(saltPepperArtifact) } });

    RingArtifact ringArtifact;
    ringArtifact.SetBrightIntensity(500.0);
    ringArtifact.SetBrightRingWidth(15.0);
    ringArtifact.SetDarkRingWidth(5.0);
    imageArtifactConcatenation.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(ringArtifact) } });

    WindMillArtifact windMillArtifact;
    windMillArtifact.SetBrightIntensity(500.0);
    windMillArtifact.SetBrightAngularWidth(15.0);
    windMillArtifact.SetDarkAngularWidth(15.0);
    imageArtifactConcatenation.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(windMillArtifact) } });

    CuppingArtifact cuppingArtifact;
    cuppingArtifact.SetDarkIntensity(-500.0);
    imageArtifactConcatenation.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(cuppingArtifact) } });

    StairStepArtifact stairStepArtifact;
    stairStepArtifact.SetRelativeZAxisSamplingRate(0.75);
    imageArtifactConcatenation.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(stairStepArtifact) } });


    PipelineGroup& pipelineGroup = PipelineGroups->AddPipelineGroup(pipeline, "MyPipelineGroup");

    auto gaussianProperties = gaussian.GetProperties();
    auto& gaussianMeanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> gaussianMeanSpan (ArtifactVariantPointer(&gaussian),
                                           gaussianMeanProperty,
                                           { gaussianMeanProperty.Get(), gaussianMeanProperty.Get() + 5, 1.0 },
                                           "My Mean Property");
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(gaussianMeanSpan));
}
