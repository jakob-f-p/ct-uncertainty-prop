#include "DataInitializer.h"

#include "App.h"
#include "Artifacts/Image/CompositeImageArtifact.h"
#include "Artifacts/Image/ImageArtifact.h"
#include "Artifacts/Image/ImageArtifactConcatenation.h"
#include "Artifacts/Structure/StructureArtifact.h"
#include "Artifacts/Structure/StructureArtifactListCollection.h"
#include "Artifacts/PipelineList.h"
#include "Modeling/BasicStructure.h"
#include "Modeling/CombinedStructure.h"
#include "Modeling/CtDataSource.h"
#include "Modeling/CtStructureTree.h"
#include "Modeling/NrrdCtDataSource.h"
#include "PipelineGroups/PipelineGroup.h"
#include "PipelineGroups/PipelineGroupList.h"
#include "PipelineGroups/PipelineParameterSpan.h"
#include "Segmentation/ThresholdFilter.h"

#include "Ui/Utils/RenderWidget.h"
#include "Utils/Statistics.h"

#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkImageAccumulate.h>
#include <vtkImageData.h>
#include <vtkPointData.h>

#include <stdexcept>
#include <numbers>
#include <QStandardPaths>


DataInitializer::DataInitializer(App& app) :
        App_(app),
        CtDataTree(app.GetCtDataTree()),
        Pipelines(app.GetPipelines()),
        PipelineGroups(app.GetPipelineGroups()) {}

auto DataInitializer::operator()(Config config) const -> void {
    switch (config) {
        case Config::DEFAULT:
            DefaultSceneInitializer{ App_ }();
            break;

        case Config::DEBUG:
            DebugSceneInitializer{ App_ }();
            break;

        case Config::DEBUG_SINGLE:
            DebugSingleSceneInitializer{ App_ }();
            break;

        case Config::SIMPLE_SCENE:
            SimpleSceneInitializer{ App_ }();
            break;

        case Config::METHODOLOGY_ACQUISITION:
            MethodologyAcquisitionSceneInitializer{ App_ }();
            break;

        case Config::METHODOLOGY_ARTIFACTS:
            MethodologyArtifactsSceneInitializer{ App_ }();
            break;

        case Config::METHODOLOGY_SEGMENTATION:
            MethodologySegmentationSceneInitializer{ App_ }();
            break;

        case Config::METHODOLOGY_ANALYSIS:
            MethodologyAnalysisSceneInitializer{ App_ }();
            break;

        case Config::SCENARIO_IMPLICIT:
            ScenarioImplicitInitializer{ App_ }();
            break;

        case Config::SCENARIO_IMPORTED:
            ScenarioImportedInitializer{ App_ }();
            break;

        case Config::WORKFLOW_FIGURE:
            WorkflowFigureInitializer{ App_ }();
            break;

        default: throw std::runtime_error("invalid config");
    }
}



SceneInitializer::SceneInitializer(App& app) :
        App_(app),
        CtDataTree(app.GetCtDataTree()),
        Pipelines(app.GetPipelines()),
        PipelineGroups(app.GetPipelineGroups()) {}


DefaultSceneInitializer::DefaultSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto DefaultSceneInitializer::operator()() const -> void {
    BasicStructure sphere(Sphere{});
    sphere.SetTissueType(BasicStructureDetails::GetTissueTypeByName("Cancellous Bone"));
    sphere.SetTransformData({ 40.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });

    BasicStructure box(Box{});
    box.SetTissueType(BasicStructureDetails::GetTissueTypeByName("Cortical Bone"));
    box.SetTransformData({ 10.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });

    CombinedStructure combinedStructure;
    combinedStructure.SetOperatorType(CombinedStructure::OperatorType::UNION);
    combinedStructure.SetTransformData({ -10.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });

    CtDataTree.AddBasicStructure(std::move(sphere));
    CtDataTree.CombineWithBasicStructure(std::move(box), std::move(combinedStructure));


    auto& pipelineA = Pipelines.AddPipeline();
    auto& pipelineB = Pipelines.AddPipeline();

    auto& structureArtifactsA = pipelineA.GetStructureArtifactList(1);
    auto& structureArtifactsB = pipelineB.GetStructureArtifactList(1);

    ImageArtifactConcatenation& imageArtifactConcatenationA = pipelineA.GetImageArtifactConcatenation();
    ImageArtifactConcatenation& imageArtifactConcatenationB = pipelineB.GetImageArtifactConcatenation();

    MotionArtifact motionArtifact {};
    motionArtifact.SetRadiodensityFactor(5.0);
    motionArtifact.SetTransform({ 1.0, 0.0, 0.0,
                                  0.0, 0.0, 0.0,
                                  1.0, 1.0, 1.0 });
    StructureArtifact motionStructureArtifact(std::move(motionArtifact));
    motionStructureArtifact.SetName("1");

    structureArtifactsA.AddStructureArtifact(std::move(motionStructureArtifact));

    BasicImageArtifact gaussianArtifact(GaussianArtifact{});
    gaussianArtifact.SetName("sequential gaussian");
    auto& gaussian = imageArtifactConcatenationA.AddImageArtifact(ImageArtifact(std::move(gaussianArtifact)));

    CompositeImageArtifact compositeImageArtifact;
    compositeImageArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    auto& composite = imageArtifactConcatenationA.AddImageArtifact(ImageArtifact(std::move(compositeImageArtifact)));

    CompositeImageArtifact compositeImageArtifact1;
    compositeImageArtifact1.SetCompositionType(CompositeImageArtifactDetails::CompositionType::SEQUENTIAL);
    auto& composite1 = imageArtifactConcatenationA.AddImageArtifact(ImageArtifact(std::move(compositeImageArtifact1)), &composite);
    BasicImageArtifact gaussianArtifact2(GaussianArtifact{});
    BasicImageArtifact gaussianArtifact3(GaussianArtifact{});
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact(std::move(gaussianArtifact2)), &composite1);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact(std::move(gaussianArtifact3)), &composite1);

    BasicImageArtifact gaussianArtifact1(GaussianArtifact{});
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact(std::move(gaussianArtifact1)), &composite);

    BasicImageArtifact gaussianArtifact4(GaussianArtifact{});
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact(std::move(gaussianArtifact4)));

    SaltPepperArtifact saltPepperArtifact;
    saltPepperArtifact.SetSaltAmount(0.001);
    saltPepperArtifact.SetSaltIntensity(1000.0);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(saltPepperArtifact) } });

    RingArtifact ringArtifact;
    ringArtifact.SetRingWidth(5.0);
    ringArtifact.SetInnerRadius(10.0);
    ringArtifact.SetRadiodensityFactor(1.75);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(ringArtifact) } });

    WindMillArtifact windMillArtifact;
    windMillArtifact.SetBrightIntensity(500.0);
    windMillArtifact.SetBrightAngularWidth(15.0);
    windMillArtifact.SetDarkAngularWidth(15.0);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(windMillArtifact) } });

    CuppingArtifact cuppingArtifact;
    cuppingArtifact.SetMinRadiodensityFactor(-500.0);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(cuppingArtifact) } });

    StairStepArtifact stairStepArtifact;
    stairStepArtifact.SetRelativeZAxisSamplingRate(0.75);
    imageArtifactConcatenationA.AddImageArtifact(ImageArtifact { BasicImageArtifact { std::move(stairStepArtifact) } });

    MotionArtifact motionArtifactB {};
    motionArtifactB.SetRadiodensityFactor(5.0);
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
    auto& gaussianB1 = imageArtifactConcatenationB.AddImageArtifact(ImageArtifact(std::move(gaussianArtifactB1)));
    CuppingArtifact cuppingArtifactBsub;
    cuppingArtifactBsub.SetMinRadiodensityFactor(-300.0);
    BasicImageArtifact cuppingArtifactB(cuppingArtifactBsub);
    cuppingArtifactB.SetName("cupping(-300.0)");
    auto& cuppingB = imageArtifactConcatenationB.AddImageArtifact(ImageArtifact(std::move(cuppingArtifactB)));


    PipelineGroup& pipelineGroupA = PipelineGroups.AddPipelineGroup(pipelineA, "PipelineGroup A");

    auto gaussianProperties = gaussian.GetProperties();
    auto& gaussianMeanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan gaussianMeanSpan (ArtifactVariantPointer(&gaussian),
                                           gaussianMeanProperty,
    { gaussianMeanProperty.Get(), gaussianMeanProperty.Get() + 750, 50.0 },
    "My Mean Property");
    pipelineGroupA.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(gaussianMeanSpan));


    PipelineGroup& pipelineGroupB = PipelineGroups.AddPipelineGroup(pipelineB, "PipelineGroup B");

    auto gaussianB1Properties = gaussianB1.GetProperties();
    auto& gaussianB1MeanProperty = gaussianB1Properties.GetPropertyByName<float>("Mean");
    ParameterSpan gaussianB1MeanSpan (ArtifactVariantPointer(&gaussianB1),
                                             gaussianB1MeanProperty,
    { gaussianB1MeanProperty.Get(),
                gaussianB1MeanProperty.Get() + 100.0F,
                50.0F },
    "Mean 1");
    pipelineGroupB.AddParameterSpan(ArtifactVariantPointer(&gaussianB1), std::move(gaussianB1MeanSpan));

    auto cuppingBProperties = cuppingB.GetProperties();
    auto& cuppingBCenterProperty = cuppingBProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan cuppingBCenterSpan (ArtifactVariantPointer(&cuppingB),
                                                  cuppingBCenterProperty,
    { cuppingBCenterProperty.Get(),
                { static_cast<float>(cuppingBCenterProperty.Get()[0] + 175.0),
                  cuppingBCenterProperty.Get()[1],
                  cuppingBCenterProperty.Get()[2] },
                { 25.0, 0.0, 0.0 } },
    "Cupping 2");
    pipelineGroupB.AddParameterSpan(ArtifactVariantPointer(&cuppingB), std::move(cuppingBCenterSpan));
}


DebugSceneInitializer::DebugSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto DebugSceneInitializer::operator()() const -> void {
    auto cancellousBoneTissueType = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdBetween(cancellousBoneTissueType.Radiodensity * 0.95,
                                     cancellousBoneTissueType.Radiodensity * 1.05);

    BasicStructure sphere(Sphere{});
    sphere.SetTissueType(cancellousBoneTissueType);
    sphere.SetTransformData({ 10.0F, 10.0F, 10.0F, 0.0F, 0.0F, 0.0F, 2.5F, 2.5F, 2.5F });

    CtDataTree.AddBasicStructure(std::move(sphere));

    static constexpr Range<float> meanRange = { -25.0,  25.0, 5.0 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(meanRange.GetCenter());
    gaussianArtifact.SetStandardDeviation(20.0);

    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
    gaussianBasicArtifact.SetName("gaussian");
    auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(gaussianBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");

    auto gaussianProperties = gaussian.GetProperties();

    auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan meanSpan {
            ArtifactVariantPointer(&gaussian),
            meanProperty,
            { meanRange.Min, meanRange.Max, meanRange.Step },
            "Mean Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));
}

DebugSingleSceneInitializer::DebugSingleSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto DebugSingleSceneInitializer::operator()() const -> void {
//    auto cancellousBoneTissueType = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");
//
//    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
//    thresholdFilter.ThresholdBetween(cancellousBoneTissueType.CtNumber * 0.95,
//                                     cancellousBoneTissueType.CtNumber * 1.05);
//
//    BasicStructure sphere(Sphere{});
//    sphere.SetTissueType(cancellousBoneTissueType);
//    sphere.SetTransformData({ 10.0F, 10.0F, 10.0F, 0.0F, 0.0F, 0.0F, 2.5F, 2.5F, 2.5F });
//
//    CtDataTree.AddBasicStructure(std::move(sphere));

    auto cancellousBoneTissueType = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdByUpper(cancellousBoneTissueType.Radiodensity * 0.95);

    BasicStructure sphere(Sphere{});
    sphere.SetTissueType(cancellousBoneTissueType);
    sphere.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 2.0F, 2.0F, 2.0F });

    CtDataTree.AddBasicStructure(std::move(sphere));

//    static Range<float> const meanRange = { 0.0,  0.0, 0.0 };
//
//    auto& pipeline = Pipelines.AddPipeline();
//
//    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();
//
//    GaussianArtifact gaussianArtifact {};
//    gaussianArtifact.SetMean(meanRange.GetCenter());
//    gaussianArtifact.SetStandardDeviation(20.0);
//
//    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
//    gaussianBasicArtifact.SetName("gaussian");
//    auto& gaussian = concatenation.AddImageArtifact(std::move(gaussianBasicArtifact));
//
//    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");
//
//    auto gaussianProperties = gaussian.GetProperties();
//
//    auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
//    ParameterSpan<float> meanSpan {
//            ArtifactVariantPointer(&gaussian),
//            meanProperty,
//            { meanRange.Min, meanRange.Max, meanRange.Step },
//            "Mean Span"
//    };
//    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));

    static constexpr Range<float> saltAmountRange = { 0.0, 0.01, 0.001 };
    static constexpr Range<float> pepperAmountRange = { 0.0, 0.01, 0.001 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    SaltPepperArtifact saltPepperArtifact {};
    saltPepperArtifact.SetSaltIntensity(1500.0F);
    saltPepperArtifact.SetPepperIntensity(-900.0F);
    saltPepperArtifact.SetSaltAmount(saltAmountRange.GetCenter());
    saltPepperArtifact.SetPepperAmount(pepperAmountRange.GetCenter());

    BasicImageArtifact saltPepperBasicArtifact { std::move(saltPepperArtifact) };
    saltPepperBasicArtifact.SetName("salt and pepper");
    auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(saltPepperBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Salt Pepper Pipelines");

    auto saltPepperProperties = saltPepper.GetProperties();

    auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
    ParameterSpan saltAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            saltAmountProperty,
            { saltAmountRange.Min, saltAmountRange.Max, saltAmountRange.Step },
            "Salt Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(saltAmountSpan));

    auto& pepperAmountProperty = saltPepperProperties.GetPropertyByName<float>("Pepper Amount");
    ParameterSpan pepperAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            pepperAmountProperty,
            { pepperAmountRange.Min, pepperAmountRange.Max, pepperAmountRange.Step },
            "Pepper Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(pepperAmountSpan));
}

SimpleSceneInitializer::SimpleSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto SimpleSceneInitializer::operator()() const -> void {
    auto const cancellousBoneTissueType = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdBetween(cancellousBoneTissueType.Radiodensity * 0.95,
                                     cancellousBoneTissueType.Radiodensity * 1.05);

    BasicStructure sphere(Sphere{});
    sphere.SetTissueType(cancellousBoneTissueType);
    sphere.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 2.0F, 2.0F, 2.0F });

    CtDataTree.AddBasicStructure(std::move(sphere));

    InitializeGaussian();
    InitializeCupping();
    InitializeRing();
    InitializeSaltPepper();
    InitializeStairStep();
    InitializeWindMill();
    InitializeMotion();
}

auto SimpleSceneInitializer::InitializeGaussian() const noexcept -> void {
    static constexpr Range<float> meanRange = { -70.0,  70.0, 10.0 };
    static constexpr Range<float> sdRange   = {   0.0, 200.0, 25.0 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(meanRange.GetCenter());
    gaussianArtifact.SetStandardDeviation(sdRange.GetCenter());

    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
    gaussianBasicArtifact.SetName("gaussian");
    auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(gaussianBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");

    auto gaussianProperties = gaussian.GetProperties();

    auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan meanSpan {
            ArtifactVariantPointer(&gaussian),
            meanProperty,
            { meanRange.Min, meanRange.Max, meanRange.Step },
            "Mean Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));

    auto& sdProperty = gaussianProperties.GetPropertyByName<float>("Standard Deviation");
    ParameterSpan sdSpan {
            ArtifactVariantPointer(&gaussian),
            sdProperty,
            { sdRange.Min, sdRange.Max, sdRange.Step },
            "Standard Deviation Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(sdSpan));
}

auto SimpleSceneInitializer::InitializeCupping() const noexcept -> void {
    static constexpr Range<float> minRadiodensityFactorRange = { 0.0, 1.0, 0.1 };
    static constexpr Range<FloatPoint> centerRange = { { -50.0, -50.0, -50.0 },
                                                   { 50.0, 50.0, 50.0 },
                                                   { 10.0, 10.0, 10.0 } };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    CuppingArtifact cuppingArtifact {};
    cuppingArtifact.SetMinRadiodensityFactor(minRadiodensityFactorRange.GetCenter());
    cuppingArtifact.SetCenter(centerRange.GetCenter());

    BasicImageArtifact cuppingBasicArtifact { std::move(cuppingArtifact) };
    cuppingBasicArtifact.SetName("cupping");
    auto& cupping = concatenation.AddImageArtifact(ImageArtifact(std::move(cuppingBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Cupping Pipelines");

    auto cuppingProperties = cupping.GetProperties();

    auto& minRadiodensityFactorProperty = cuppingProperties.GetPropertyByName<float>("Minimum Radiodensity Factor");
    ParameterSpan minRadiodensityFactorSpan {
            ArtifactVariantPointer(&cupping),
            minRadiodensityFactorProperty,
            { minRadiodensityFactorRange.Min, minRadiodensityFactorRange.Max, minRadiodensityFactorRange.Step },
            "Minimum Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&cupping), std::move(minRadiodensityFactorSpan));

    auto& centerProperty = cuppingProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan centerSpan {
            ArtifactVariantPointer(&cupping),
            centerProperty,
            { centerRange.Min, centerRange.Max, centerRange.Step },
            "Center Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&cupping), std::move(centerSpan));
}

auto SimpleSceneInitializer::InitializeRing() const noexcept -> void {
    static constexpr Range<float> radiodensityFactorRange = { 0.25, 1.75, 0.125 };
    static constexpr Range<FloatPoint> centerRange = { { -50.0, -50.0, -50.0 },
                                                   { 50.0, 50.0, 50.0 },
                                                   { 10.0, 10.0, 10.0 } };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    RingArtifact ringArtifact {};
    ringArtifact.SetInnerRadius(5.0F);
    ringArtifact.SetRingWidth(2.0F);
    ringArtifact.SetRadiodensityFactor(radiodensityFactorRange.Max);
    ringArtifact.SetCenter(centerRange.GetCenter());

    BasicImageArtifact ringBasicArtifact { std::move(ringArtifact) };
    ringBasicArtifact.SetName("ring");
    auto& ring = concatenation.AddImageArtifact(ImageArtifact(std::move(ringBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Ring Pipelines");

    auto ringProperties = ring.GetProperties();

    auto& radiodensityFactorProperty = ringProperties.GetPropertyByName<float>("Radiodensity Factor");
    ParameterSpan radiodensityFactorSpan {
            ArtifactVariantPointer(&ring),
            radiodensityFactorProperty,
            { radiodensityFactorRange.Min, radiodensityFactorRange.Max, radiodensityFactorRange.Step },
            "Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(radiodensityFactorSpan));

    auto& centerProperty = ringProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan centerSpan {
            ArtifactVariantPointer(&ring),
            centerProperty,
            { centerRange.Min, centerRange.Max, centerRange.Step },
            "Center Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(centerSpan));
}

auto SimpleSceneInitializer::InitializeSaltPepper() const noexcept -> void {
    static constexpr Range<float> saltAmountRange = { 0.0, 0.01, 0.001 };
    static constexpr Range<float> pepperAmountRange = { 0.0, 0.01, 0.001 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    SaltPepperArtifact saltPepperArtifact {};
    saltPepperArtifact.SetSaltIntensity(1500.0F);
    saltPepperArtifact.SetPepperIntensity(-900.0F);
    saltPepperArtifact.SetSaltAmount(saltAmountRange.GetCenter());
    saltPepperArtifact.SetPepperAmount(pepperAmountRange.GetCenter());

    BasicImageArtifact saltPepperBasicArtifact { std::move(saltPepperArtifact) };
    saltPepperBasicArtifact.SetName("salt and pepper");
    auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(saltPepperBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Salt Pepper Pipelines");

    auto saltPepperProperties = saltPepper.GetProperties();

    auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
    ParameterSpan saltAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            saltAmountProperty,
            { saltAmountRange.Min, saltAmountRange.Max, saltAmountRange.Step },
            "Salt Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(saltAmountSpan));

    auto& pepperAmountProperty = saltPepperProperties.GetPropertyByName<float>("Pepper Amount");
    ParameterSpan pepperAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            pepperAmountProperty,
            { pepperAmountRange.Min, pepperAmountRange.Max, pepperAmountRange.Step },
            "Pepper Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(pepperAmountSpan));
}

auto SimpleSceneInitializer::InitializeStairStep() const noexcept -> void {
    static constexpr Range<float> zSamplingRate = { 0.1, 1.0, 0.008 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    StairStepArtifact stairStepArtifact {};
    stairStepArtifact.SetRelativeZAxisSamplingRate(zSamplingRate.GetCenter());

    BasicImageArtifact stairStepBasicArtifact { std::move(stairStepArtifact) };
    stairStepBasicArtifact.SetName("stair step");
    auto& stairStep = concatenation.AddImageArtifact(ImageArtifact(std::move(stairStepBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Stair Step Pipelines");

    auto stairStepProperties = stairStep.GetProperties();

    auto& zAxisSamplingRateProperty = stairStepProperties.GetPropertyByName<float>("z-Axis Sampling Rate");
    ParameterSpan zAxisSamplingRateSpan {
            ArtifactVariantPointer(&stairStep),
            zAxisSamplingRateProperty,
            { zSamplingRate.Min, zSamplingRate.Max, zSamplingRate.Step },
            "z-Axis Sampling Rate Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&stairStep), std::move(zAxisSamplingRateSpan));
}

auto SimpleSceneInitializer::InitializeWindMill() const noexcept -> void {
    static constexpr Range<float> brightIntensityRange = { 0.0, 80.0, 10.0 };
    static constexpr Range<float> angularWidth = { 1.0, 13.0, 4.0 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    WindMillArtifact windMillArtifact {};
    windMillArtifact.SetCenter({ 0, 0, 0 });
    windMillArtifact.SetDarkIntensity(0.0);
    windMillArtifact.SetBrightIntensity(brightIntensityRange.GetCenter());
    windMillArtifact.SetDarkAngularWidth(angularWidth.GetCenter());
    windMillArtifact.SetBrightAngularWidth(angularWidth.GetCenter());

    BasicImageArtifact windMillBasicArtifact { std::move(windMillArtifact) };
    windMillBasicArtifact.SetName("wind mill");
    auto& windMill = concatenation.AddImageArtifact(ImageArtifact(std::move(windMillBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Wind Mill Pipelines");

    auto windMillProperties = windMill.GetProperties();

    auto& brightIntensityProperty = windMillProperties.GetPropertyByName<float>("Bright Intensity");
    ParameterSpan brightIntensitySpan {
            ArtifactVariantPointer(&windMill),
            brightIntensityProperty,
            { brightIntensityRange.Min, brightIntensityRange.Max, brightIntensityRange.Step },
            "Bright Intensity Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&windMill), std::move(brightIntensitySpan));

    auto& brightAngularWidthProperty = windMillProperties.GetPropertyByName<float>("Bright Angular Width");
    ParameterSpan brightAngularWidthSpan {
            ArtifactVariantPointer(&windMill),
            brightAngularWidthProperty,
            { angularWidth.Min, angularWidth.Max, angularWidth.Step },
            "Bright Angular Width Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&windMill), std::move(brightAngularWidthSpan));

    auto& darkAngularWidthProperty = windMillProperties.GetPropertyByName<float>("Dark Angular Width");
    ParameterSpan darkAngularWidthSpan {
            ArtifactVariantPointer(&windMill),
            darkAngularWidthProperty,
            { angularWidth.Min, angularWidth.Max, angularWidth.Step },
            "Dark Angular Width Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&windMill), std::move(darkAngularWidthSpan));
}

auto SimpleSceneInitializer::InitializeMotion() const noexcept -> void {
    static constexpr Range<float> ctNumberFactorRange = { 0.5, 1.5, 0.01 };

    auto& pipeline = Pipelines.AddPipeline();

    auto& structureArtifacts = pipeline.GetStructureArtifactList(0);

    MotionArtifact motionArtifact {};
    motionArtifact.SetRadiodensityFactor(ctNumberFactorRange.GetCenter());
    motionArtifact.SetTransform({ 3.0, -3.0, -3.0,
                                  0.0, 0.0, 0.0,
                                  1.1, 1.0, 1.0 });
    StructureArtifact motionStructureArtifact(std::move(motionArtifact));
    motionStructureArtifact.SetName("motion");
    structureArtifacts.AddStructureArtifact(std::move(motionStructureArtifact));
    auto& motion = structureArtifacts.Get(0);

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Motion Pipelines");

    auto motionProperties = motion.GetProperties();

    auto& radiodensityFactorProperty = motionProperties.GetPropertyByName<float>("Radiodensity Factor");
    ParameterSpan radiodensityFactorSpan {
            ArtifactVariantPointer(&motion),
            radiodensityFactorProperty,
            { ctNumberFactorRange.Min, ctNumberFactorRange.Max, ctNumberFactorRange.Step },
            "Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&motion), std::move(radiodensityFactorSpan));
}



MethodologyAcquisitionSceneInitializer::MethodologyAcquisitionSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto MethodologyAcquisitionSceneInitializer::operator()() const -> void {
    auto airTissue = BasicStructureDetails::GetTissueTypeByName("Air");
    auto lungTissue = BasicStructureDetails::GetTissueTypeByName("Lung");
    auto muscleTissue = BasicStructureDetails::GetTissueTypeByName("Muscle");
    auto cancellousBoneTissue = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");
    auto corticalBoneTissue   = BasicStructureDetails::GetTissueTypeByName("Cortical Bone");

    CtRenderWidget::SetWindowWidth({ 0.0, corticalBoneTissue.Radiodensity });

    auto& dataSource = App_.GetCtDataSource();
    dataSource.SetVolumeDataPhysicalDimensions({ 40.0, 40.0, 40.0 });
#ifdef BUILD_TYPE_DEBUG
    dataSource.SetVolumeNumberOfVoxels({ 64, 64, 32 });
#else
    dataSource.SetVolumeNumberOfVoxels({ 256, 256, 128 });
#endif

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdBetween(-1500.0, 900.0);

    Cylinder spineCylinder {};
    spineCylinder.SetFunctionData({ 3.0, 37.5 });
    BasicStructure spineCylinderStructure { std::move(spineCylinder) };
    spineCylinderStructure.SetTissueType(corticalBoneTissue);
    spineCylinderStructure.SetTransformData({ 0.0F, -20.0F, -8.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    spineCylinderStructure.SetEvaluationBias(-100.0F);
    spineCylinderStructure.SetName("Spine");
    CtDataTree.AddBasicStructure(std::move(spineCylinderStructure));

    Cylinder ribCageInnerCylinder {};
    ribCageInnerCylinder.SetFunctionData({ 10.0, 30.0 });
    BasicStructure ribCageAirCylinderStructure { std::move(ribCageInnerCylinder) };
    ribCageAirCylinderStructure.SetTissueType(airTissue);
    ribCageAirCylinderStructure.SetTransformData({ 0.0F, -15.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.5F, 1.0F, 1.0F });
    ribCageAirCylinderStructure.SetName("Inner");

    CombinedStructure sceneUnion { CombinedStructure::OperatorType::UNION };
    sceneUnion.SetName("Chest");
    sceneUnion.SetTransformData({ 0.0F, 0.0F, 0.0F, 90.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    CtDataTree.CombineWithBasicStructure(std::move(ribCageAirCylinderStructure), std::move(sceneUnion));

    Cylinder ribCageOuterCylinder {};
    ribCageOuterCylinder.SetFunctionData({ 12.0, 30.0 });
    BasicStructure ribCageCylinderStructure { std::move(ribCageOuterCylinder) };
    ribCageCylinderStructure.SetTissueType(corticalBoneTissue);
    ribCageCylinderStructure.SetTransformData({ 0.0F, -15.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.5F, 1.0F, 1.0F });
    ribCageCylinderStructure.SetName("Outer");

    CombinedStructure ribCage { CombinedStructure::OperatorType::DIFFERENCE_ };
    ribCage.SetName("Rib Cage");
    CtDataTree.RefineWithBasicStructure(std::move(ribCageCylinderStructure), std::move(ribCage), 1);


    Box rightLung {};
    rightLung.SetFunctionData({ Point { 2.0, -12.0, -7.0 }, Point { 12.0, 16.0, 7.0 } });
    BasicStructure rightLungStructure { std::move(rightLung) };
    rightLungStructure.SetTissueType(lungTissue);
    rightLungStructure.SetEvaluationBias(-100.0F);
    rightLungStructure.SetName("Right");

    auto* chestStructure = &std::get<CombinedStructure>(CtDataTree.GetRoot());
    CtDataTree.AddBasicStructure(std::move(rightLungStructure), chestStructure);

    Box leftLung {};
    leftLung.SetFunctionData({ Point { -12.0, -12.0, -7.0 }, Point { -2.0, 16.0, 7.0 } });
    BasicStructure leftLungStructure { std::move(leftLung) };
    leftLungStructure.SetTissueType(lungTissue);
    leftLungStructure.SetEvaluationBias(-100.0F);
    leftLungStructure.SetName("Left");

    CombinedStructure lungsUnion { CombinedStructure::OperatorType::UNION };
    lungsUnion.SetName("Lungs");
    CtDataTree.RefineWithBasicStructure(std::move(leftLungStructure), std::move(lungsUnion), 1);


    Sphere heartSphere {};
    heartSphere.SetFunctionData({ 5.0, Point { 2.0, 5.0, 2.0 } });
    BasicStructure heartSphereStructure { std::move(heartSphere) };
    heartSphereStructure.SetTissueType(muscleTissue);
    heartSphereStructure.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.3F, 0.9F });
    heartSphereStructure.SetEvaluationBias(-200.0F);
    heartSphereStructure.SetName("Heart");

    chestStructure = &std::get<CombinedStructure>(CtDataTree.GetRoot());
    CtDataTree.AddBasicStructure(std::move(heartSphereStructure), chestStructure);


    static constexpr Range<float> meanRange = { -25.0,  25.0, 5.0 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(meanRange.GetCenter());
    gaussianArtifact.SetStandardDeviation(20.0);

    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
    gaussianBasicArtifact.SetName("gaussian");
    auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(gaussianBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");

    auto gaussianProperties = gaussian.GetProperties();

    auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan meanSpan {
            ArtifactVariantPointer(&gaussian),
            meanProperty,
            { meanRange.Min, meanRange.Max, meanRange.Step },
            "Mean Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));
}



MethodologyArtifactsSceneInitializer::MethodologyArtifactsSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto MethodologyArtifactsSceneInitializer::operator()() const -> void {
    auto softTissue = BasicStructureDetails::GetTissueTypeByName("Soft Tissue");
    auto cancellousBoneTissue = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

    CtRenderWidget::SetWindowWidth({ -100.0, cancellousBoneTissue.Radiodensity + 100.0 });

    auto& dataSource = App_.GetCtDataSource();
#ifdef BUILD_TYPE_DEBUG
    dataSource.SetVolumeNumberOfVoxels({ 32, 32, 16 });
#else
    dataSource.SetVolumeNumberOfVoxels({ 128, 128, 64 });
#endif

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdByUpper(cancellousBoneTissue.Radiodensity - 100.0F);


    Sphere sphere {};
    sphere.SetFunctionData({ 25.0, { -15.0, 10.0, 10.0 } });
    BasicStructure sphereStructure { std::move(sphere) };
    sphereStructure.SetTissueType(softTissue);
    sphereStructure.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    CtDataTree.AddBasicStructure(std::move(sphereStructure));

    Box box {};
    box.SetFunctionData({ { -5.0, -20.0, -20.0 }, { 35.0, 20.0, 20.0 } });
    BasicStructure boxStructure { std::move(box) };
    boxStructure.SetTissueType(cancellousBoneTissue);
    boxStructure.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });

    CombinedStructure sphereBoxUnion { CombinedStructure::OperatorType::UNION };
    sphereBoxUnion.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    CtDataTree.CombineWithBasicStructure(std::move(boxStructure), std::move(sphereBoxUnion));


    auto& pipeline = Pipelines.Get(0);

    MotionArtifact unionMotionArtifact {};
    unionMotionArtifact.SetRadiodensityFactor(0.7);
    unionMotionArtifact.SetBlurKernelStandardDeviation(1.0);
    unionMotionArtifact.SetBlurKernelRadius(2);
#ifdef BUILD_TYPE_DEBUG
    unionMotionArtifact.SetBlurKernelRadius(2);
#else
    unionMotionArtifact.SetBlurKernelRadius(4);
#endif
    unionMotionArtifact.SetTransform({ 4.0F, 4.0F, 4.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    StructureArtifact unionMotionStructureArtifact { std::move(unionMotionArtifact) };
    auto& unionStructureArtifacts = pipeline.GetStructureArtifactList(0);
    unionStructureArtifacts.AddStructureArtifact(std::move(unionMotionStructureArtifact));

    MotionArtifact boxMotionArtifact {};
    boxMotionArtifact.SetRadiodensityFactor(0.8);
    boxMotionArtifact.SetBlurKernelStandardDeviation(1.0);
    boxMotionArtifact.SetBlurKernelRadius(0);
    boxMotionArtifact.SetTransform({ -6.0F, -4.0F, -4.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    StructureArtifact boxMotionStructureArtifact { std::move(boxMotionArtifact) };
    auto& boxStructureArtifacts = pipeline.GetStructureArtifactList(2);
    boxStructureArtifacts.AddStructureArtifact(std::move(boxMotionStructureArtifact));


    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    CompositeImageArtifact parallelCompositeArtifact;
    parallelCompositeArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    parallelCompositeArtifact.SetName("sequential");
    auto& compositeArtifact = concatenation.AddImageArtifact(ImageArtifact(std::move(parallelCompositeArtifact)));

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(10.0);
    gaussianArtifact.SetStandardDeviation(30.0);
    BasicImageArtifact gaussianBasicArtifact(std::move(gaussianArtifact));
    gaussianBasicArtifact.SetName("parallel");
    concatenation.AddImageArtifact(ImageArtifact(std::move(gaussianBasicArtifact)), &compositeArtifact);

    RingArtifact ringArtifact {};
    ringArtifact.SetRadiodensityFactor(0.3);
    ringArtifact.SetInnerRadius(10.0);
    ringArtifact.SetRingWidth(3.0);
    BasicImageArtifact ringBasicArtifact(std::move(ringArtifact));
    ringBasicArtifact.SetName("parallel");
    concatenation.AddImageArtifact(ImageArtifact(std::move(ringBasicArtifact)), &compositeArtifact);

    SaltPepperArtifact saltPepperArtifact {};
#ifdef BUILD_TYPE_DEBUG
    saltPepperArtifact.SetSaltAmount(0.02);
#else
    saltPepperArtifact.SetSaltAmount(0.07);
#endif
    saltPepperArtifact.SetSaltIntensity(450);
    BasicImageArtifact saltPepperBasicArtifact(std::move(saltPepperArtifact));
    saltPepperBasicArtifact.SetName("sequential");
    concatenation.AddImageArtifact(ImageArtifact(std::move(saltPepperBasicArtifact)));
}

MethodologySegmentationSceneInitializer::MethodologySegmentationSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto MethodologySegmentationSceneInitializer::operator()() const -> void {
    auto softTissue = BasicStructureDetails::GetTissueTypeByName("Soft Tissue");
    auto cancellousBoneTissue = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

    CtRenderWidget::SetWindowWidth({ -100.0, cancellousBoneTissue.Radiodensity + 100.0 });

    auto& dataSource = App_.GetCtDataSource();
#ifdef BUILD_TYPE_DEBUG
    dataSource.SetVolumeNumberOfVoxels({ 32, 32, 16 });
#else
    dataSource.SetVolumeNumberOfVoxels({ 128, 128, 64 });
#endif

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdByUpper(300.0);


    Sphere sphere {};
    sphere.SetFunctionData({ 25.0, { -15.0, 10.0, 10.0 } });
    BasicStructure sphereStructure { std::move(sphere) };
    sphereStructure.SetTissueType(softTissue);
    sphereStructure.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    CtDataTree.AddBasicStructure(std::move(sphereStructure));

    Box box {};
    box.SetFunctionData({ { -5.0, -20.0, -20.0 }, { 35.0, 20.0, 20.0 } });
    BasicStructure boxStructure { std::move(box) };
    boxStructure.SetTissueType(cancellousBoneTissue);
    boxStructure.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });

    CombinedStructure sphereBoxUnion { CombinedStructure::OperatorType::UNION };
    sphereBoxUnion.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    CtDataTree.CombineWithBasicStructure(std::move(boxStructure), std::move(sphereBoxUnion));


    auto& pipeline = Pipelines.Get(0);

    MotionArtifact unionMotionArtifact {};
    unionMotionArtifact.SetRadiodensityFactor(0.7);
    unionMotionArtifact.SetBlurKernelStandardDeviation(1.0);
    unionMotionArtifact.SetBlurKernelRadius(2);
#ifdef BUILD_TYPE_DEBUG
    unionMotionArtifact.SetBlurKernelRadius(2);
#else
    unionMotionArtifact.SetBlurKernelRadius(4);
#endif
    unionMotionArtifact.SetTransform({ 4.0F, 4.0F, 4.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    StructureArtifact unionMotionStructureArtifact { std::move(unionMotionArtifact) };
    auto& unionStructureArtifacts = pipeline.GetStructureArtifactList(0);
    unionStructureArtifacts.AddStructureArtifact(std::move(unionMotionStructureArtifact));

    MotionArtifact boxMotionArtifact {};
    boxMotionArtifact.SetRadiodensityFactor(0.8);
    boxMotionArtifact.SetBlurKernelStandardDeviation(1.0);
    boxMotionArtifact.SetBlurKernelRadius(0);
    boxMotionArtifact.SetTransform({ -6.0F, -4.0F, -4.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    StructureArtifact boxMotionStructureArtifact { std::move(boxMotionArtifact) };
    auto& boxStructureArtifacts = pipeline.GetStructureArtifactList(2);
    boxStructureArtifacts.AddStructureArtifact(std::move(boxMotionStructureArtifact));


    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    CompositeImageArtifact parallelCompositeArtifact;
    parallelCompositeArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    parallelCompositeArtifact.SetName("sequential");
    auto& compositeArtifact = concatenation.AddImageArtifact(ImageArtifact(std::move(parallelCompositeArtifact)));

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(10.0);
    gaussianArtifact.SetStandardDeviation(30.0);
    BasicImageArtifact gaussianBasicArtifact(std::move(gaussianArtifact));
    gaussianBasicArtifact.SetName("parallel");
    concatenation.AddImageArtifact(ImageArtifact(std::move(gaussianBasicArtifact)), &compositeArtifact);

    RingArtifact ringArtifact {};
    ringArtifact.SetRadiodensityFactor(0.3);
    ringArtifact.SetInnerRadius(10.0);
    ringArtifact.SetRingWidth(3.0);
    BasicImageArtifact ringBasicArtifact(std::move(ringArtifact));
    ringBasicArtifact.SetName("parallel");
    concatenation.AddImageArtifact(ImageArtifact(std::move(ringBasicArtifact)), &compositeArtifact);

    SaltPepperArtifact saltPepperArtifact {};
#ifdef BUILD_TYPE_DEBUG
    saltPepperArtifact.SetSaltAmount(0.02);
#else
    saltPepperArtifact.SetSaltAmount(0.07);
#endif
    saltPepperArtifact.SetSaltIntensity(450);
    BasicImageArtifact saltPepperBasicArtifact(std::move(saltPepperArtifact));
    saltPepperBasicArtifact.SetName("sequential");
    concatenation.AddImageArtifact(ImageArtifact(std::move(saltPepperBasicArtifact)));
}



MethodologyAnalysisSceneInitializer::MethodologyAnalysisSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto MethodologyAnalysisSceneInitializer::operator()() const -> void {
    auto softTissue = BasicStructureDetails::GetTissueTypeByName("Soft Tissue");
    auto cancellousBoneTissue = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

    CtRenderWidget::SetWindowWidth({ -100.0, cancellousBoneTissue.Radiodensity + 100.0 });

    auto& dataSource = App_.GetCtDataSource();
#ifdef BUILD_TYPE_DEBUG
    dataSource.SetVolumeNumberOfVoxels({ 32, 32, 16 });
#else
    dataSource.SetVolumeNumberOfVoxels({ 128, 128, 64 });
#endif

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdByUpper(300.0);


    Sphere sphere {};
    sphere.SetFunctionData({ 25.0, { -15.0, 10.0, 10.0 } });
    BasicStructure sphereStructure { std::move(sphere) };
    sphereStructure.SetTissueType(softTissue);
    sphereStructure.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    CtDataTree.AddBasicStructure(std::move(sphereStructure));

    Box box {};
    box.SetFunctionData({ { -5.0, -20.0, -20.0 }, { 35.0, 20.0, 20.0 } });
    BasicStructure boxStructure { std::move(box) };
    boxStructure.SetTissueType(cancellousBoneTissue);
    boxStructure.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });

    CombinedStructure sphereBoxUnion { CombinedStructure::OperatorType::UNION };
    sphereBoxUnion.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    CtDataTree.CombineWithBasicStructure(std::move(boxStructure), std::move(sphereBoxUnion));


    auto& pipeline = Pipelines.Get(0);

    MotionArtifact unionMotionArtifact {};
    unionMotionArtifact.SetRadiodensityFactor(0.7);
    unionMotionArtifact.SetBlurKernelStandardDeviation(1.0);
    unionMotionArtifact.SetBlurKernelRadius(2);
#ifdef BUILD_TYPE_DEBUG
    unionMotionArtifact.SetBlurKernelRadius(2);
#else
    unionMotionArtifact.SetBlurKernelRadius(4);
#endif
    unionMotionArtifact.SetTransform({ 4.0F, 4.0F, 4.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    StructureArtifact unionMotionStructureArtifact { std::move(unionMotionArtifact) };
    auto& unionStructureArtifacts = pipeline.GetStructureArtifactList(0);
    unionStructureArtifacts.AddStructureArtifact(std::move(unionMotionStructureArtifact));

    MotionArtifact boxMotionArtifact {};
    boxMotionArtifact.SetRadiodensityFactor(0.8);
    boxMotionArtifact.SetBlurKernelStandardDeviation(1.0);
    boxMotionArtifact.SetBlurKernelRadius(0);
    boxMotionArtifact.SetTransform({ -6.0F, -4.0F, -4.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    StructureArtifact boxMotionStructureArtifact { std::move(boxMotionArtifact) };
    auto& boxStructureArtifacts = pipeline.GetStructureArtifactList(2);
    boxStructureArtifacts.AddStructureArtifact(std::move(boxMotionStructureArtifact));


    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    CompositeImageArtifact parallelCompositeArtifact;
    parallelCompositeArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    parallelCompositeArtifact.SetName("sequential");
    auto& compositeArtifact = concatenation.AddImageArtifact(ImageArtifact(std::move(parallelCompositeArtifact)));

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(10.0);
    gaussianArtifact.SetStandardDeviation(30.0);
    BasicImageArtifact gaussianBasicArtifact(std::move(gaussianArtifact));
    gaussianBasicArtifact.SetName("parallel");
    auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(gaussianBasicArtifact)), &compositeArtifact);

    RingArtifact ringArtifact {};
    ringArtifact.SetRadiodensityFactor(0.3);
    ringArtifact.SetInnerRadius(10.0);
    ringArtifact.SetRingWidth(3.0);
    BasicImageArtifact ringBasicArtifact(std::move(ringArtifact));
    ringBasicArtifact.SetName("parallel");
    concatenation.AddImageArtifact(ImageArtifact(std::move(ringBasicArtifact)), &compositeArtifact);

    SaltPepperArtifact saltPepperArtifact {};
#ifdef BUILD_TYPE_DEBUG
    saltPepperArtifact.SetSaltAmount(0.02);
#else
    saltPepperArtifact.SetSaltAmount(0.07);
#endif
    saltPepperArtifact.SetSaltIntensity(450);
    BasicImageArtifact saltPepperBasicArtifact(std::move(saltPepperArtifact));
    saltPepperBasicArtifact.SetName("sequential");
    auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(saltPepperBasicArtifact)));


    {
        PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Pipelines A");

        static constexpr Range<float> meanRange = { -10.0, 10.0, 5.0 };
        static constexpr Range<float> sdRange = { 0.0, 40.0, 10.0 };

        auto gaussianProperties = gaussian.GetProperties();
        auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
        ParameterSpan meanSpan{
                ArtifactVariantPointer(&gaussian),
                meanProperty,
                { meanRange.Min, meanRange.Max, meanRange.Step },
                "Mean Span"
        };
        pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));

        auto& sdProperty = gaussianProperties.GetPropertyByName<float>("Standard Deviation");
        ParameterSpan sdSpan{
                ArtifactVariantPointer(&gaussian),
                sdProperty,
                { sdRange.Min, sdRange.Max, sdRange.Step },
                "Standard Deviation Span"
        };
        pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(sdSpan));


        auto& unionMotion = unionStructureArtifacts.Get(0);

        auto motionProperties = unionMotion.GetProperties();

        static constexpr Range<float> radiodensityFactorRange = { 0.5, 1.5, 0.25 };
        auto& radiodensityFactorProperty = motionProperties.GetPropertyByName<float>("Radiodensity Factor");
        ParameterSpan radiodensityFactorSpan {
                ArtifactVariantPointer(&unionMotion),
                radiodensityFactorProperty,
                { radiodensityFactorRange.Min, radiodensityFactorRange.Max, radiodensityFactorRange.Step },
                "Radiodensity Factor Span"
        };
        pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&unionMotion), std::move(radiodensityFactorSpan));
    }


    {
        PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Pipelines B");

        static constexpr Range<float> saltAmountRange = { 0.0, 0.20, 0.0025 };

        auto saltPepperProperties = saltPepper.GetProperties();

        auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
        ParameterSpan saltAmountSpan {
                ArtifactVariantPointer(&saltPepper),
                saltAmountProperty,
                { saltAmountRange.Min, saltAmountRange.Max, saltAmountRange.Step },
                "Salt Amount Span"
        };
        pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(saltAmountSpan));
    }
}

ScenarioImplicitInitializer::ScenarioImplicitInitializer(App& app) :
        SceneInitializer(app) {}

auto ScenarioImplicitInitializer::operator()() const -> void {
    auto waterTissue = BasicStructureDetails::GetTissueTypeByName("Water");
    auto organ1Tissue = BasicStructureDetails::GetTissueTypeByName("Organ1");
    auto organ2Tissue = BasicStructureDetails::GetTissueTypeByName("Organ2");
    auto metalTissue   = BasicStructureDetails::GetTissueTypeByName("Metal");

    CtRenderWidget::SetWindowWidth({ waterTissue.Radiodensity - 25.0F, organ2Tissue.Radiodensity + 25.0F });

    auto& dataSource = App_.GetCtDataSource();
    dataSource.SetVolumeDataPhysicalDimensions({ 40.0, 40.0, 20.0 });
#ifdef BUILD_TYPE_DEBUG
//    dataSource.SetVolumeNumberOfVoxels({ 64, 64, 32 });
    dataSource.SetVolumeNumberOfVoxels({ 16, 16, 16 });
#else
//    dataSource.SetVolumeNumberOfVoxels({ 256, 256, 128 });
    dataSource.SetVolumeNumberOfVoxels({ 128, 128, 64 });
#endif

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdBetween(organ1Tissue.Radiodensity - 15.0F, organ1Tissue.Radiodensity + 15.0F);

    Cylinder waterCylinder {};
    waterCylinder.SetFunctionData({ 15.0, 20.0 });
    BasicStructure waterCylinderStructure { std::move(waterCylinder) };
    waterCylinderStructure.SetTissueType(waterTissue);
    waterCylinderStructure.SetTransformData({ 0.0F, 0.0F, -10.0F, 0.0F, 0.0F, 0.0F, 1.3F, 1.0F, 1.0F });
    waterCylinderStructure.SetName("Water");
    CtDataTree.AddBasicStructure(std::move(waterCylinderStructure));

    CombinedStructure sceneUnion { CombinedStructure::OperatorType::UNION };
    sceneUnion.SetName("Scene");

    Sphere organSphere {};
    organSphere.SetFunctionData({ 10.0, {} });
    BasicStructure organSphereStructure { std::move(organSphere) };
    organSphereStructure.SetTissueType(organ1Tissue);
    organSphereStructure.SetTransformData({ -5.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    organSphereStructure.SetName("Organ1");
    organSphereStructure.SetEvaluationBias(-2000.0F);
    CtDataTree.CombineWithBasicStructure(std::move(organSphereStructure), std::move(sceneUnion));

    Box organBox {};
    organBox.SetFunctionData({ { 0.0F, 0.0F, 0.0F }, { 15.0F, 15.0F, 15.0F } });
    BasicStructure organBoxStructure { std::move(organBox) };
    organBoxStructure.SetTissueType(organ2Tissue);
    organBoxStructure.SetTransformData({ -2.5F, -10.0F, -7.5F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    organBoxStructure.SetName("Organ2");
    organBoxStructure.SetEvaluationBias(-1000.0F);
    auto* sceneRoot = &std::get<CombinedStructure>(CtDataTree.GetRoot());
    CtDataTree.AddBasicStructure(std::move(organBoxStructure), sceneRoot);

    Sphere metalSphere {};
    metalSphere.SetFunctionData({ 3.0, {} });
    BasicStructure metalSphereStructure { std::move(metalSphere) };
    metalSphereStructure.SetTissueType(metalTissue);
    metalSphereStructure.SetTransformData({ 4.0F, 5.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    metalSphereStructure.SetName("Metal");
    metalSphereStructure.SetEvaluationBias(-3000.0F);
    sceneRoot = &std::get<CombinedStructure>(CtDataTree.GetRoot());
    CtDataTree.AddBasicStructure(std::move(metalSphereStructure), sceneRoot);

    InitializeSaltPepper();
    InitializeGaussian();
    InitializeCupping();
    InitializeRing();
    InitializeMetal();
    InitializeWindmill();
    InitializeMotion();
}

auto ScenarioImplicitInitializer::InitializeSaltPepper() const noexcept -> void {
    static constexpr Range<float> saltAmountRange = { 0.005, 0.05, 0.005 };
    static constexpr Range<float> pepperAmountRange = { 0.005, 0.05, 0.005 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    SaltPepperArtifact saltPepperArtifact {};
    saltPepperArtifact.SetSaltIntensity(1500.0F);
    saltPepperArtifact.SetPepperIntensity(-900.0F);
    saltPepperArtifact.SetSaltAmount(saltAmountRange.GetCenter());
    saltPepperArtifact.SetPepperAmount(pepperAmountRange.GetCenter());

    BasicImageArtifact saltPepperBasicArtifact { std::move(saltPepperArtifact) };
    auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(saltPepperBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Salt Pepper Pipelines");

    auto saltPepperProperties = saltPepper.GetProperties();

    auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
    ParameterSpan saltAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            saltAmountProperty,
            { saltAmountRange.Min, saltAmountRange.Max, saltAmountRange.Step },
            "Salt Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(saltAmountSpan));

    auto& pepperAmountProperty = saltPepperProperties.GetPropertyByName<float>("Pepper Amount");
    ParameterSpan pepperAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            pepperAmountProperty,
            { pepperAmountRange.Min, pepperAmountRange.Max, pepperAmountRange.Step },
            "Pepper Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(pepperAmountSpan));
}

auto ScenarioImplicitInitializer::InitializeGaussian() const noexcept -> void {
    static constexpr Range<float> sdRange   = { 0.0, 20.0, 0.2 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(0.0);
    gaussianArtifact.SetStandardDeviation(sdRange.GetCenter());

    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
    auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(gaussianBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");

    auto gaussianProperties = gaussian.GetProperties();

    auto& sdProperty = gaussianProperties.GetPropertyByName<float>("Standard Deviation");
    ParameterSpan sdSpan {
            ArtifactVariantPointer(&gaussian),
            sdProperty,
            { sdRange.Min, sdRange.Max, sdRange.Step },
            "Standard Deviation Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(sdSpan));
}

auto ScenarioImplicitInitializer::InitializeCupping() const noexcept -> void {
    static constexpr Range<float> minRadiodensityFactorRange = { 0.5, 0.95, 0.05 };
    static constexpr Range<FloatPoint> centerRange = { { -10.0, -10.0, -10.0 },
                                                   { 10.0, 10.0, 10.0 },
                                                   { 2.0, 2.0, 2.0 } };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    CuppingArtifact cuppingArtifact {};
    cuppingArtifact.SetMinRadiodensityFactor(minRadiodensityFactorRange.GetCenter());
    cuppingArtifact.SetCenter(centerRange.GetCenter());

    BasicImageArtifact cuppingBasicArtifact { std::move(cuppingArtifact) };
    auto& cupping = concatenation.AddImageArtifact(ImageArtifact(std::move(cuppingBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Cupping Pipelines");

    auto cuppingProperties = cupping.GetProperties();

    auto& minRadiodensityFactorProperty = cuppingProperties.GetPropertyByName<float>("Minimum Radiodensity Factor");
    ParameterSpan minRadiodensityFactorSpan {
            ArtifactVariantPointer(&cupping),
            minRadiodensityFactorProperty,
            { minRadiodensityFactorRange.Min, minRadiodensityFactorRange.Max, minRadiodensityFactorRange.Step },
            "Minimum Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&cupping), std::move(minRadiodensityFactorSpan));

    auto& centerProperty = cuppingProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan centerSpan {
            ArtifactVariantPointer(&cupping),
            centerProperty,
            { centerRange.Min, centerRange.Max, centerRange.Step },
            "Center Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&cupping), std::move(centerSpan));
}

auto ScenarioImplicitInitializer::InitializeRing() const noexcept -> void {
    static constexpr Range<float> radiodensityFactorRange = { 0.0, 2.0, 0.20 };
    static constexpr Range<float> innerRadiusRange = { 0.0, 10.0, 1.0 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    RingArtifact ringArtifact {};
    ringArtifact.SetInnerRadius(innerRadiusRange.GetCenter());
    ringArtifact.SetRingWidth(2.0F);
    ringArtifact.SetRadiodensityFactor(radiodensityFactorRange.Max);
    ringArtifact.SetCenter({});

    BasicImageArtifact ringBasicArtifact { std::move(ringArtifact) };
    auto& ring = concatenation.AddImageArtifact(ImageArtifact(std::move(ringBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Ring Pipelines");

    auto ringProperties = ring.GetProperties();

    auto& radiodensityFactorProperty = ringProperties.GetPropertyByName<float>("Radiodensity Factor");
    ParameterSpan radiodensityFactorSpan {
            ArtifactVariantPointer(&ring),
            radiodensityFactorProperty,
            { radiodensityFactorRange.Min, radiodensityFactorRange.Max, radiodensityFactorRange.Step },
            "Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(radiodensityFactorSpan));

    auto& innerRadiusProperty = ringProperties.GetPropertyByName<float>("Inner Radius");
    ParameterSpan innerRadiusSpan {
            ArtifactVariantPointer(&ring),
            innerRadiusProperty,
            { innerRadiusRange.Min, innerRadiusRange.Max, innerRadiusRange.Step },
            "Inner Radius Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(innerRadiusSpan));
}

auto ScenarioImplicitInitializer::InitializeMetal() const noexcept -> void {
    static constexpr Range<float> attenuationFactorRange = { 0.0, 0.9, 0.1 };
    static constexpr Range<float> lengthRange = { 1.0, 10.0, 1.0 };

    auto& pipeline = Pipelines.AddPipeline();

    auto& structureArtifacts = pipeline.GetStructureArtifactList(1);

    MetalArtifact metalArtifact {};
    metalArtifact.SetLength(lengthRange.GetCenter());
    metalArtifact.SetMaxAttenuationFactor(attenuationFactorRange.GetCenter());
    metalArtifact.SetDirectionOfHighestAttenuation({ -1.0, 0.0 });
    StructureArtifact metalStructureArtifact(std::move(metalArtifact));
    structureArtifacts.AddStructureArtifact(std::move(metalStructureArtifact));
    auto& metal = structureArtifacts.Get(0);

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Metal Pipelines");

    auto metalProperties = metal.GetProperties();

    auto& attenuationFactorProperty = metalProperties.GetPropertyByName<float>("Maximum Attenuation Factor");
    ParameterSpan attenuationFactorSpan {
            ArtifactVariantPointer(&metal),
            attenuationFactorProperty,
            { attenuationFactorRange.Min, attenuationFactorRange.Max, attenuationFactorRange.Step },
            "Attenuation Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&metal), std::move(attenuationFactorSpan));

    auto& lengthProperty = metalProperties.GetPropertyByName<float>("Length");
    ParameterSpan lengthSpan {
            ArtifactVariantPointer(&metal),
            lengthProperty,
            { lengthRange.Min, lengthRange.Max, lengthRange.Step },
            "Length Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&metal), std::move(lengthSpan));
}

auto ScenarioImplicitInitializer::InitializeWindmill() const noexcept -> void {
    static constexpr Range<float> angularWidthRange = { 30.0, 90.0, 6.0 };
    static constexpr Range<float> lengthRange = { 1.0, 10.0, 1.0 };

    auto& pipeline = Pipelines.AddPipeline();

    auto& structureArtifacts = pipeline.GetStructureArtifactList(1);

    WindmillArtifact windmillArtifact {};
    windmillArtifact.SetRotationPerSlice(std::numbers::pi / 180);
    windmillArtifact.SetRadiodensityFactor(1.0);
    windmillArtifact.SetAngularWidth(angularWidthRange.GetCenter() * std::numbers::pi / 180);
    windmillArtifact.SetLength(lengthRange.GetCenter());
    StructureArtifact windmillStructureArtifact { std::move(windmillArtifact) };
    structureArtifacts.AddStructureArtifact(std::move(windmillStructureArtifact));
    auto& metal = structureArtifacts.Get(0);

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Windmill Pipelines");

    auto metalProperties = metal.GetProperties();

    auto& angularWidthProperty = metalProperties.GetPropertyByName<float>("Angular Width");
    ParameterSpan angularWidthSpan {
            ArtifactVariantPointer(&metal),
            angularWidthProperty,
            { angularWidthRange.Min, angularWidthRange.Max, angularWidthRange.Step },
            "Angular Width Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&metal), std::move(angularWidthSpan));

    auto& lengthProperty = metalProperties.GetPropertyByName<float>("Length");
    ParameterSpan lengthSpan {
            ArtifactVariantPointer(&metal),
            lengthProperty,
            { lengthRange.Min, lengthRange.Max, lengthRange.Step },
            "Length Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&metal), std::move(lengthSpan));
}

auto ScenarioImplicitInitializer::InitializeMotion() const noexcept -> void {
    static constexpr Range<float> radiodensityFactorRange = { 0.9, 1.1, 0.02 };
    static constexpr Range<float> blurSdRange = { 0.1, 10.0, 1.0 };
    auto& pipeline = Pipelines.AddPipeline();

    auto& structureArtifacts = pipeline.GetStructureArtifactList(3);

    MotionArtifact motionArtifact {};
    motionArtifact.SetTransform({ 3.0, -3.0, -3.0,
                                  0.0, 0.0, 0.0,
                                  1.1, 1.0, 1.0 });
    motionArtifact.SetBlurKernelRadius(3);
    motionArtifact.SetRadiodensityFactor(radiodensityFactorRange.GetCenter());
    motionArtifact.SetBlurKernelStandardDeviation(blurSdRange.GetCenter());
    StructureArtifact motionStructureArtifact { std::move(motionArtifact) };
    structureArtifacts.AddStructureArtifact(std::move(motionStructureArtifact));
    auto& motion = structureArtifacts.Get(0);

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Motion Pipelines");

    auto motionProperties = motion.GetProperties();

    auto& radiodensityFactorProperty = motionProperties.GetPropertyByName<float>("Radiodensity Factor");
    ParameterSpan radiodensityFactorSpan {
            ArtifactVariantPointer(&motion),
            radiodensityFactorProperty,
            { radiodensityFactorRange.Min, radiodensityFactorRange.Max, radiodensityFactorRange.Step },
            "Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&motion), std::move(radiodensityFactorSpan));

    auto& blurSdProperty = motionProperties.GetPropertyByName<float>("Blur Standard Deviation");
    ParameterSpan blurSdSpan {
            ArtifactVariantPointer(&motion),
            blurSdProperty,
            { blurSdRange.Min, blurSdRange.Max, blurSdRange.Step },
            "Blur Standard Deviation Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&motion), std::move(blurSdSpan));
}



ScenarioImportedInitializer::ScenarioImportedInitializer(App& app) :
        SceneInitializer(app) {}

auto ScenarioImportedInitializer::operator()() const -> void {
    vtkNew<NrrdCtDataSource> dataSource;
    dataSource->SetVolumeDataPhysicalDimensions({ 100.0, 100.0, 100.0 });
#ifdef BUILD_TYPE_DEBUG
    dataSource->SetVolumeNumberOfVoxels({ 64, 64, 64 });
//    dataSource->SetVolumeNumberOfVoxels({ 16, 16, 16 });
#else
    dataSource->SetVolumeNumberOfVoxels({ 255, 255, 255 });
//    dataSource->SetVolumeNumberOfVoxels({ 128, 128, 128 });
#endif
    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    std::filesystem::path const& homePath { homeLocations.at(0).toStdString() };
    std::filesystem::path const& filename = homePath / "Downloads/aneurism.nhdr";
    std::filesystem::path const& abs_filepath = absolute(filename);
    dataSource->SetFilepath(abs_filepath);
    App_.SetCtDataSource(*dataSource);

    App_.GetCtDataSource().Update();
    vtkImageData& inputImage = *App_.GetCtDataSource().GetOutput();

    auto* radiodensityArray = vtkFloatArray::SafeDownCast(inputImage.GetPointData()->GetScalars());
    float* radiodensities = radiodensityArray->WritePointer(0, inputImage.GetNumberOfPoints());
    std::span const radiodensitySpan { radiodensities, static_cast<size_t>(inputImage.GetNumberOfPoints()) };
    auto const [ imageMinIt, imageMaxIt ] = std::minmax_element(radiodensitySpan.begin(), radiodensitySpan.end());
    CtRenderWidget::SetWindowWidth({ *imageMinIt - 30.0, *imageMaxIt });
//    CtRenderWidget::SetWindowWidth({ *imageMinIt - 300.0, (*imageMaxIt)*2.0 }); // brighter

    double const threshold = CalculateOtsuThreshold(*App_.GetCtDataSource().GetOutput());
    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdByUpper(threshold);

    {
        auto& pipeline = Pipelines.Get(0);

        ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

        auto [saltPepperBasicArtifact, gaussianBasicArtifact, cuppingBasicArtifact, ringBasicArtifact] = CreateBasicImageArtifacts();

        CompositeImageArtifact physicsArtifacts;
        physicsArtifacts.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
        auto& compositeArtifact = concatenation.AddImageArtifact(ImageArtifact(std::move(physicsArtifacts)));
        auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(*gaussianBasicArtifact.release())),
                                                        &compositeArtifact);
        auto& cupping = concatenation.AddImageArtifact(ImageArtifact(std::move(*cuppingBasicArtifact.release())),
                                                       &compositeArtifact);

        auto& ring = concatenation.AddImageArtifact(ImageArtifact(std::move(*ringBasicArtifact.release())));

        auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(*saltPepperBasicArtifact.release())));

        PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Order A");

        ImageArtifacts imageArtifacts { saltPepper, gaussian, cupping, ring };

        AddParameterSpans(pipelineGroup, imageArtifacts);
    }
    {
        auto& pipeline = Pipelines.AddPipeline();

        ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

        auto [saltPepperBasicArtifact, gaussianBasicArtifact, cuppingBasicArtifact, ringBasicArtifact] = CreateBasicImageArtifacts();

        CompositeImageArtifact physicsArtifacts;
        physicsArtifacts.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
        auto& compositeArtifact = concatenation.AddImageArtifact(ImageArtifact(std::move(physicsArtifacts)));
        auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(*saltPepperBasicArtifact.release())),
                                                          &compositeArtifact);
        auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(*gaussianBasicArtifact.release())),
                                                        &compositeArtifact);
        auto& cupping = concatenation.AddImageArtifact(ImageArtifact(std::move(*cuppingBasicArtifact.release())),
                                                       &compositeArtifact);
        auto& ring = concatenation.AddImageArtifact(ImageArtifact(std::move(*ringBasicArtifact.release())),
                                                    &compositeArtifact);

        PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Order B");

        ImageArtifacts imageArtifacts { saltPepper, gaussian, cupping, ring };

        AddParameterSpans(pipelineGroup, imageArtifacts);
    }{
        auto& pipeline = Pipelines.AddPipeline();

        ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

        auto [saltPepperBasicArtifact, gaussianBasicArtifact, cuppingBasicArtifact, ringBasicArtifact] = CreateBasicImageArtifacts();

        auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(*saltPepperBasicArtifact.release())));
        auto& gaussian = concatenation.AddImageArtifact(ImageArtifact(std::move(*gaussianBasicArtifact.release())));
        auto& cupping = concatenation.AddImageArtifact(ImageArtifact(std::move(*cuppingBasicArtifact.release())));
        auto& ring = concatenation.AddImageArtifact(ImageArtifact(std::move(*ringBasicArtifact.release())));

        PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Order C");

        ImageArtifacts imageArtifacts { saltPepper, gaussian, cupping, ring };

        AddParameterSpans(pipelineGroup, imageArtifacts);
    }
}

auto ScenarioImportedInitializer::CreateBasicImageArtifacts() -> BasicImageArtifacts {
    SaltPepperArtifact saltPepperArtifact {};
    saltPepperArtifact.SetSaltIntensity(3000.0F);
    saltPepperArtifact.SetPepperIntensity(-1000.0F);
    saltPepperArtifact.SetSaltAmount(SaltAmountRange.GetCenter());
    saltPepperArtifact.SetPepperAmount(PepperAmountRange.GetCenter());
    BasicImageArtifact saltPepperBasicArtifact { std::move(saltPepperArtifact) };
    saltPepperBasicArtifact.SetName("transmission-based");

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(0.0);
    gaussianArtifact.SetStandardDeviation(GaussianSdRange.GetCenter());
    BasicImageArtifact gaussianBasicArtifact(std::move(gaussianArtifact));
    gaussianBasicArtifact.SetName("physics-based");

    CuppingArtifact cuppingArtifact {};
    cuppingArtifact.SetMinRadiodensityFactor(CuppingMinRdFactorRange.GetCenter());
    cuppingArtifact.SetCenter({ 0.0, 0.0, 0.0 });
    BasicImageArtifact cuppingBasicArtifact { std::move(cuppingArtifact) };
    cuppingBasicArtifact.SetName("physics-based");

    RingArtifact ringArtifact {};
    ringArtifact.SetInnerRadius(RingInnerRadiusRange.GetCenter());
    ringArtifact.SetRingWidth(5.0F);
    ringArtifact.SetRadiodensityFactor(2.0);
    ringArtifact.SetCenter({ 0.0, 0.0, 0.0 });
    BasicImageArtifact ringBasicArtifact { std::move(ringArtifact) };
    ringBasicArtifact.SetName("scanner-based");

    return { std::make_unique<BasicImageArtifact>(std::move(saltPepperBasicArtifact)),
             std::make_unique<BasicImageArtifact>(std::move(gaussianBasicArtifact)),
             std::make_unique<BasicImageArtifact>(std::move(cuppingBasicArtifact)),
             std::make_unique<BasicImageArtifact>(std::move(ringBasicArtifact ))};
}

auto ScenarioImportedInitializer::AddParameterSpans(PipelineGroup& pipelineGroup, ImageArtifacts& artifacts) -> void {
    auto gaussianProperties = artifacts.Gaussian.GetProperties();
    auto& sdProperty = gaussianProperties.GetPropertyByName<float>("Standard Deviation");
    ParameterSpan sdSpan {
            ArtifactVariantPointer(&artifacts.Gaussian),
            sdProperty,
            { GaussianSdRange.Min, GaussianSdRange.Max, GaussianSdRange.Step },
            "Standard Deviation Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&artifacts.Gaussian), std::move(sdSpan));

    auto cuppingProperties = artifacts.Cupping.GetProperties();
    auto& minRadiodensityFactorProperty = cuppingProperties.GetPropertyByName<float>("Minimum Radiodensity Factor");
    ParameterSpan minRadiodensityFactorSpan {
            ArtifactVariantPointer(&artifacts.Cupping),
            minRadiodensityFactorProperty,
            { CuppingMinRdFactorRange.Min, CuppingMinRdFactorRange.Max, CuppingMinRdFactorRange.Step },
            "Minimum Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&artifacts.Cupping), std::move(minRadiodensityFactorSpan));

    auto ringProperties = artifacts.Ring.GetProperties();
    auto& innerRadiusProperty = ringProperties.GetPropertyByName<float>("Inner Radius");
    ParameterSpan innerRadiusSpan {
            ArtifactVariantPointer(&artifacts.Ring),
            innerRadiusProperty,
            { RingInnerRadiusRange.Min, RingInnerRadiusRange.Max, RingInnerRadiusRange.Step },
            "Inner Radius Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&artifacts.Ring), std::move(innerRadiusSpan));

    auto saltPepperProperties = artifacts.SaltPepper.GetProperties();
    auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
    auto& pepperAmountProperty = saltPepperProperties.GetPropertyByName<float>("Pepper Amount");
    ParameterSpan saltAmountSpan {
            ArtifactVariantPointer(&artifacts.SaltPepper),
            saltAmountProperty,
            { SaltAmountRange.Min, SaltAmountRange.Max, SaltAmountRange.Step },
            "Salt Amount Span"
    };
    ParameterSpan pepperAmountSpan {
            ArtifactVariantPointer(&artifacts.SaltPepper),
            pepperAmountProperty,
            { PepperAmountRange.Min, PepperAmountRange.Max, PepperAmountRange.Step },
            "Pepper Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&artifacts.SaltPepper), std::move(saltAmountSpan));
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&artifacts.SaltPepper), std::move(pepperAmountSpan));
}

auto ScenarioImportedInitializer::CalculateOtsuThreshold(vtkImageData& image) -> double {
    size_t const numberOfValues = image.GetNumberOfPoints();

    auto* radiodensityArray = vtkFloatArray::SafeDownCast(image.GetPointData()->GetScalars());
    float* radiodensities = radiodensityArray->WritePointer(0, numberOfValues);
    std::span const radiodensitySpan { radiodensities, numberOfValues };

    auto const [ imageMinIt, imageMaxIt ] = std::minmax_element(radiodensitySpan.begin(), radiodensitySpan.end());
    double const min = std::max(floor(static_cast<double>(*imageMinIt)), -1000.0);
    double const max = std::min(ceil(static_cast<double>(*imageMaxIt)),  3000.0);
    int const numberOfBins = max - min + 1;

    vtkNew<vtkImageAccumulate> const histogramFilter;
    histogramFilter->SetComponentSpacing(1.0, 0.0, 0.0);
    histogramFilter->SetComponentOrigin(min, 0.0, 0.0);
    histogramFilter->SetComponentExtent(0, numberOfBins - 1, 0, 0, 0, 0);
    histogramFilter->SetInputData(&image);
    histogramFilter->Update();
    auto* histogramImage = histogramFilter->GetOutput();

    auto* histogramArray = vtkIdTypeArray::SafeDownCast(histogramImage->GetPointData()->GetScalars());
    vtkIdType* histogramValues = histogramArray->WritePointer(0, histogramImage->GetNumberOfPoints());
    std::span const histogramSpan { histogramValues,
                                                  static_cast<size_t>(histogramImage->GetNumberOfPoints()) };
    std::vector<double> relativeHistogramVector { histogramSpan.size(), std::allocator<double> {} };
    std::ranges::transform(histogramSpan,
                           relativeHistogramVector.begin(),
                           [numberOfValues](vtkIdType const& val) {
                               return static_cast<double>(val) / static_cast<double>(numberOfValues);
                           });

    std::vector<double> cdfVector = Stats::CumulativeSum(relativeHistogramVector);
    assert(abs(cdfVector.back() - 1.0) < 1e-10);
    cdfVector.back() = 1.0;

    auto SumOfProducts = [](std::span<double> const& relativeHistogram, double initialValue) -> double {
        double sum = 0.0;
        double value = initialValue;

        for (double const& p : relativeHistogram) {
            sum += p * value;
            ++value;
        }

        return sum;
    };

    auto CalculateInterClassVariance = [](double p0, double mu0, double p1, double mu1) -> double {
        double const& meanDifference = mu1 - mu0;
        return p0 * p1 * meanDifference * meanDifference;
    };

    int optimalThreshold = 0;
    double maxInterClassVariance = -1.0;
    for (int i = 1, threshold = min + 1.0; i < numberOfBins - 1; ++i, ++threshold) {
        auto const cutoffIt = std::next(relativeHistogramVector.begin(), i);
        std::span const class1 { relativeHistogramVector.begin(), cutoffIt };
        std::span const class2 { cutoffIt, relativeHistogramVector.end() };
        double const interClassVariance = CalculateInterClassVariance(
                cdfVector[i], SumOfProducts(class1, min) / cdfVector[i],
                1.0 - cdfVector[i], SumOfProducts(class2, threshold) / (1.0 - cdfVector[i])
        );

        if (interClassVariance > maxInterClassVariance) {
            maxInterClassVariance = interClassVariance;
            optimalThreshold = threshold;
        }
    }

    return optimalThreshold;
}

WorkflowFigureInitializer::WorkflowFigureInitializer(App& app)
        : SceneInitializer(app) {}

auto WorkflowFigureInitializer::operator()() const -> void {
    auto waterTissue = BasicStructureDetails::GetTissueTypeByName("Water");
    auto organ1Tissue = BasicStructureDetails::GetTissueTypeByName("Organ1");
    auto organ2Tissue = BasicStructureDetails::GetTissueTypeByName("Organ2");
    auto metalTissue   = BasicStructureDetails::GetTissueTypeByName("Metal");

    CtRenderWidget::SetWindowWidth({ waterTissue.Radiodensity - 25.0F, organ2Tissue.Radiodensity + 25.0F });

    auto& dataSource = App_.GetCtDataSource();
    dataSource.SetVolumeDataPhysicalDimensions({ 40.0, 40.0, 20.0 });
#ifdef BUILD_TYPE_DEBUG
    //    dataSource.SetVolumeNumberOfVoxels({ 64, 64, 32 });
    dataSource.SetVolumeNumberOfVoxels({ 16, 16, 16 });
#else
    dataSource.SetVolumeNumberOfVoxels({ 256, 256, 128 });
//    dataSource.SetVolumeNumberOfVoxels({ 128, 128, 64 });
#endif

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdBetween(organ1Tissue.Radiodensity - 15.0F, organ1Tissue.Radiodensity + 15.0F);

    Cylinder waterCylinder {};
    waterCylinder.SetFunctionData({ 15.0, 20.0 });
    BasicStructure waterCylinderStructure { std::move(waterCylinder) };
    waterCylinderStructure.SetTissueType(waterTissue);
    waterCylinderStructure.SetTransformData({ 0.0F, 0.0F, -10.0F, 0.0F, 0.0F, 0.0F, 1.3F, 1.0F, 1.0F });
    waterCylinderStructure.SetName("Water");
    CtDataTree.AddBasicStructure(std::move(waterCylinderStructure));

    CombinedStructure sceneUnion { CombinedStructure::OperatorType::UNION };
    sceneUnion.SetName("Scene");

    Sphere organSphere {};
    organSphere.SetFunctionData({ 10.0, {} });
    BasicStructure organSphereStructure { std::move(organSphere) };
    organSphereStructure.SetTissueType(organ1Tissue);
    organSphereStructure.SetTransformData({ -5.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    organSphereStructure.SetName("Organ1");
    organSphereStructure.SetEvaluationBias(-2000.0F);
    CtDataTree.CombineWithBasicStructure(std::move(organSphereStructure), std::move(sceneUnion));

    Box organBox {};
    organBox.SetFunctionData({ { 0.0F, 0.0F, 0.0F }, { 15.0F, 15.0F, 15.0F } });
    BasicStructure organBoxStructure { std::move(organBox) };
    organBoxStructure.SetTissueType(organ2Tissue);
    organBoxStructure.SetTransformData({ -2.5F, -10.0F, -7.5F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    organBoxStructure.SetName("Organ2");
    organBoxStructure.SetEvaluationBias(-1000.0F);
    auto* sceneRoot = &std::get<CombinedStructure>(CtDataTree.GetRoot());
    CtDataTree.AddBasicStructure(std::move(organBoxStructure), sceneRoot);

    Sphere metalSphere {};
    metalSphere.SetFunctionData({ 3.0, {} });
    BasicStructure metalSphereStructure { std::move(metalSphere) };
    metalSphereStructure.SetTissueType(metalTissue);
    metalSphereStructure.SetTransformData({ 4.0F, 5.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F });
    metalSphereStructure.SetName("Metal");
    metalSphereStructure.SetEvaluationBias(-3000.0F);
    sceneRoot = &std::get<CombinedStructure>(CtDataTree.GetRoot());
    CtDataTree.AddBasicStructure(std::move(metalSphereStructure), sceneRoot);

    InitializeSaltPepper();
    InitializeRing();
}

auto WorkflowFigureInitializer::InitializeSaltPepper() const noexcept -> void {
    static constexpr Range<float> saltAmountRange = { 0.00, 0.10, 0.02 };
    static constexpr Range<float> pepperAmountRange = { 0.00, 0.10, 0.02 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    SaltPepperArtifact saltPepperArtifact {};
    saltPepperArtifact.SetSaltIntensity(1500.0F);
    saltPepperArtifact.SetPepperIntensity(-900.0F);
    saltPepperArtifact.SetSaltAmount(saltAmountRange.GetCenter());
    saltPepperArtifact.SetPepperAmount(pepperAmountRange.GetCenter());

    BasicImageArtifact saltPepperBasicArtifact { std::move(saltPepperArtifact) };
    auto& saltPepper = concatenation.AddImageArtifact(ImageArtifact(std::move(saltPepperBasicArtifact)));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Salt Pepper Pipelines");

    auto saltPepperProperties = saltPepper.GetProperties();

    auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
    ParameterSpan saltAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            saltAmountProperty,
            { saltAmountRange.Min, saltAmountRange.Max, saltAmountRange.Step },
            "Salt Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(saltAmountSpan));

    auto& pepperAmountProperty = saltPepperProperties.GetPropertyByName<float>("Pepper Amount");
    ParameterSpan pepperAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            pepperAmountProperty,
            { pepperAmountRange.Min, pepperAmountRange.Max, pepperAmountRange.Step },
            "Pepper Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(pepperAmountSpan));
}

auto WorkflowFigureInitializer::InitializeRing() const noexcept -> void {
    static constexpr Range<float> innerRadiusRange = { 0.0, 10.0, 0.5 };
    static constexpr Range<float> rdFactorRange = { 0.0, 1.0, 0.05 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    RingArtifact ringArtifact {};
    ringArtifact.SetInnerRadius(innerRadiusRange.GetCenter());
    ringArtifact.SetRingWidth(2.0F);
    ringArtifact.SetRadiodensityFactor(0.5F);
    ringArtifact.SetCenter({});

    BasicImageArtifact ringBasicArtifact { std::move(ringArtifact) };
    auto& ring = concatenation.AddImageArtifact(ImageArtifact(std::move(ringBasicArtifact)));

    PipelineGroup& radiusPipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Ring Radius Pipelines");
    auto ringProperties = ring.GetProperties();
    auto& innerRadiusProperty = ringProperties.GetPropertyByName<float>("Inner Radius");
    ParameterSpan innerRadiusSpan {
            ArtifactVariantPointer(&ring),
            innerRadiusProperty,
            { innerRadiusRange.Min, innerRadiusRange.Max, innerRadiusRange.Step },
            "Inner Radius Span"
    };
    radiusPipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(innerRadiusSpan));

    PipelineGroup& rdPipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Ring Radiodensity Factor Pipelines");
    auto& rdProperty = ringProperties.GetPropertyByName<float>("Radiodensity Factor");
    ParameterSpan rdSpan {
            ArtifactVariantPointer(&ring),
            rdProperty,
            { rdFactorRange.Min, rdFactorRange.Max, rdFactorRange.Step },
            "Radiodensity Factor Span"
    };
    rdPipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(rdSpan));

//    PipelineGroup& centerPipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Ring Center Pipelines");
//    auto& centerProperty = ringProperties.GetPropertyByName<FloatPoint>("Center");
//    ParameterSpan<FloatPoint> centerSpan {
//            ArtifactVariantPointer(&ring),
//            centerProperty,
//            { centerRange.Min, centerRange.Max, centerRange.Step },
//            "Center Span"
//    };
//    centerPipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(centerSpan));
}
