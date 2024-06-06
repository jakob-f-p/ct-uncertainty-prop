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


    auto& pipelineA = Pipelines->AddPipeline();
    auto& pipelineB = Pipelines->AddPipeline();

    auto& structureArtifactsA = pipelineA.GetStructureArtifactList(1);
    auto& structureArtifactsB = pipelineB.GetStructureArtifactList(1);

    ImageArtifactConcatenation& imageArtifactConcatenationA = pipelineA.GetImageArtifactConcatenation();
    ImageArtifactConcatenation& imageArtifactConcatenationB = pipelineB.GetImageArtifactConcatenation();

    MotionArtifact motionArtifact {};
    motionArtifact.SetCtNumberFactor(5.0);
    motionArtifact.SetTransform({ 1.0, 0.0, 0.0,
                                  0.0, 0.0, 0.0,
                                  1.0, 1.0, 1.0 });
    StructureArtifact motionStructureArtifact(std::move(motionArtifact));
    motionStructureArtifact.SetName("1");

    structureArtifactsA.AddStructureArtifact(std::move(motionStructureArtifact));

    BasicImageArtifact gaussianArtifact(GaussianArtifact{});
    gaussianArtifact.SetName("sequential gaussian");
    auto& gaussian = imageArtifactConcatenationA.AddImageArtifact(std::move(gaussianArtifact));

    CompositeImageArtifact compositeImageArtifact;
    compositeImageArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    auto& composite = imageArtifactConcatenationA.AddImageArtifact(std::move(compositeImageArtifact));

    CompositeImageArtifact compositeImageArtifact1;
    compositeImageArtifact1.SetCompositionType(CompositeImageArtifactDetails::CompositionType::SEQUENTIAL);
    auto& composite1 = imageArtifactConcatenationA.AddImageArtifact(std::move(compositeImageArtifact1), &composite);
    BasicImageArtifact gaussianArtifact2(GaussianArtifact{});
    BasicImageArtifact gaussianArtifact3(GaussianArtifact{});
    imageArtifactConcatenationA.AddImageArtifact(std::move(gaussianArtifact2), &composite1);
    imageArtifactConcatenationA.AddImageArtifact(std::move(gaussianArtifact3), &composite1);

    BasicImageArtifact gaussianArtifact1(GaussianArtifact{});
    imageArtifactConcatenationA.AddImageArtifact(std::move(gaussianArtifact1), &composite);

    BasicImageArtifact gaussianArtifact4(GaussianArtifact{});
    imageArtifactConcatenationA.AddImageArtifact(std::move(gaussianArtifact4));

    SaltPepperArtifact saltPepperArtifact;
    saltPepperArtifact.SetSaltAmount(0.001);
    saltPepperArtifact.SetSaltIntensity(1000.0);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(saltPepperArtifact) } });

    RingArtifact ringArtifact;
    ringArtifact.SetBrightIntensity(500.0);
    ringArtifact.SetBrightRingWidth(15.0);
    ringArtifact.SetDarkRingWidth(5.0);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(ringArtifact) } });

    WindMillArtifact windMillArtifact;
    windMillArtifact.SetBrightIntensity(500.0);
    windMillArtifact.SetBrightAngularWidth(15.0);
    windMillArtifact.SetDarkAngularWidth(15.0);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(windMillArtifact) } });

    CuppingArtifact cuppingArtifact;
    cuppingArtifact.SetDarkIntensity(-500.0);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(cuppingArtifact) } });

    StairStepArtifact stairStepArtifact;
    stairStepArtifact.SetRelativeZAxisSamplingRate(0.75);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(stairStepArtifact) } });

    MotionArtifact motionArtifactB {};
    motionArtifactB.SetCtNumberFactor(5.0);
    motionArtifactB.SetTransform({ 1.0, 0.0, 0.0,
                                  0.0, 0.0, 0.0,
                                  1.0, 1.0, 1.0 });
    StructureArtifact motionStructureArtifactB(std::move(motionArtifactB));
    motionStructureArtifactB.SetName("1");

    structureArtifactsB.AddStructureArtifact(std::move(motionStructureArtifactB));

    GaussianArtifact gaussianArtifactB1sub {};
    gaussianArtifactB1sub.SetMean(100.0);
    gaussianArtifactB1sub.SetStandardDeviation(20.0);
    BasicImageArtifact gaussianArtifactB1(gaussianArtifactB1sub);
    gaussianArtifactB1.SetName("gaussian(100.0, 20.0)");
    auto& gaussianB1 = imageArtifactConcatenationB.AddImageArtifact(std::move(gaussianArtifactB1));
    CuppingArtifact cuppingArtifactBsub;
    cuppingArtifactBsub.SetDarkIntensity(-200.0);
    BasicImageArtifact cuppingArtifactB(cuppingArtifactBsub);
    cuppingArtifactB.SetName("cupping(-200.0)");
    auto& cuppingB = imageArtifactConcatenationB.AddImageArtifact(std::move(cuppingArtifactB));


    PipelineGroup& pipelineGroupA = PipelineGroups->AddPipelineGroup(pipelineA, "PipelineGroup A");

    auto gaussianProperties = gaussian.GetProperties();
    auto& gaussianMeanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> gaussianMeanSpan (ArtifactVariantPointer(&gaussian),
                                           gaussianMeanProperty,
                                           { gaussianMeanProperty.Get(), gaussianMeanProperty.Get() + 5, 1.0 },
                                           "My Mean Property");
    pipelineGroupA.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(gaussianMeanSpan));


    PipelineGroup& pipelineGroupB = PipelineGroups->AddPipelineGroup(pipelineB, "PipelineGroup B");

    auto gaussianB1Properties = gaussianB1.GetProperties();
    auto& gaussianB1MeanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> gaussianB1MeanSpan (ArtifactVariantPointer(&gaussianB1),
                                             gaussianMeanProperty,
                                             { gaussianMeanProperty.Get(), gaussianMeanProperty.Get() + 2, 1.0 },
                                             "Mean 1");
    pipelineGroupB.AddParameterSpan(ArtifactVariantPointer(&gaussianB1), std::move(gaussianB1MeanSpan));

    auto cuppingBProperties = cuppingB.GetProperties();
    auto& cuppingBCenterProperty = cuppingBProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan<FloatPoint> cuppingBCenterSpan (ArtifactVariantPointer(&cuppingB),
                                                  cuppingBCenterProperty,
                                                  { cuppingBCenterProperty.Get(),
                                                    { static_cast<float>(cuppingBCenterProperty.Get()[0] + 2.0),
                                                      cuppingBCenterProperty.Get()[1],
                                                      cuppingBCenterProperty.Get()[2] },
                                               1.0 },
                                             "Cupping 2");
    pipelineGroupB.AddParameterSpan(ArtifactVariantPointer(&cuppingB), std::move(cuppingBCenterSpan));
}
