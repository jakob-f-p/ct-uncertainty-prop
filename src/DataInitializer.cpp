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
#include "PipelineGroups/PipelineGroup.h"
#include "PipelineGroups/PipelineGroupList.h"
#include "PipelineGroups/PipelineParameterSpan.h"
#include "Segmentation/ThresholdFilter.h"

#include "Ui/Utils/RenderWidget.h"

#include <stdexcept>


DataInitializer::DataInitializer(App& app) :
        App_(app),
        CtDataTree(app.GetCtDataTree()),
        Pipelines(app.GetPipelines()),
        PipelineGroups(app.GetPipelineGroups()) {}

auto DataInitializer::operator()(DataInitializer::Config config) -> void {
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

        case Config::SPHERE:
            InitializeSpherical();
            break;

        case Config::SIMPLE_SCENE:
            SimpleSceneInitializer{ App_ }();
            break;

        case Config::METHODOLOGY:
            MethodologySceneInitializer{ App_ }();
            break;

        default: throw std::runtime_error("invalid config");
    }
}

auto DataInitializer::InitializeSpherical() noexcept -> void {
    BasicStructure sphere(Sphere{});
    sphere.SetTissueType(BasicStructureDetails::GetTissueTypeByName("Cancellous Bone"));
    sphere.SetTransformData({ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 2.0F, 2.0F, 2.0F });

    CtDataTree.AddBasicStructure(std::move(sphere));

    auto& pipelineGaussian = Pipelines.AddPipeline();

    ImageArtifactConcatenation& imageArtifactConcatenationGaussian
            = pipelineGaussian.GetImageArtifactConcatenation();

    BasicImageArtifact gaussianArtifact(GaussianArtifact{});
    gaussianArtifact.SetName("sequential gaussian");
    auto& gaussian = imageArtifactConcatenationGaussian.AddImageArtifact(std::move(gaussianArtifact));

    CompositeImageArtifact compositeImageArtifact;
    compositeImageArtifact.SetCompositionType(CompositeImageArtifactDetails::CompositionType::PARALLEL);
    auto& composite = imageArtifactConcatenationGaussian.AddImageArtifact(std::move(compositeImageArtifact));

    CompositeImageArtifact compositeImageArtifact1;
    compositeImageArtifact1.SetCompositionType(CompositeImageArtifactDetails::CompositionType::SEQUENTIAL);
    auto& composite1 = imageArtifactConcatenationGaussian.AddImageArtifact(std::move(compositeImageArtifact1),
                                                                           &composite);
    BasicImageArtifact gaussianArtifact2(GaussianArtifact{});
    BasicImageArtifact gaussianArtifact3(GaussianArtifact{});
    imageArtifactConcatenationGaussian.AddImageArtifact(std::move(gaussianArtifact2), &composite1);
    imageArtifactConcatenationGaussian.AddImageArtifact(std::move(gaussianArtifact3), &composite1);

    BasicImageArtifact gaussianArtifact1(GaussianArtifact{});
    imageArtifactConcatenationGaussian.AddImageArtifact(std::move(gaussianArtifact1), &composite);

    BasicImageArtifact gaussianArtifact4(GaussianArtifact{});
    imageArtifactConcatenationGaussian.AddImageArtifact(std::move(gaussianArtifact4));

    PipelineGroup& pipelineGroupA = PipelineGroups.AddPipelineGroup(pipelineGaussian, "PipelineGroup A");

    auto gaussianProperties = gaussian.GetProperties();
    auto& gaussianMeanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> gaussianMeanSpan (ArtifactVariantPointer(&gaussian),
                                           gaussianMeanProperty,
                                           { gaussianMeanProperty.Get(), gaussianMeanProperty.Get() + 250, 50.0 },
                                           "My Mean Property");
    pipelineGroupA.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(gaussianMeanSpan));
}



SceneInitializer::SceneInitializer(App& app) :
        App_(app),
        CtDataTree(app.GetCtDataTree()),
        Pipelines(app.GetPipelines()),
        PipelineGroups(app.GetPipelineGroups()) {}


DefaultSceneInitializer::DefaultSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto DefaultSceneInitializer::operator()() -> void {
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
    auto& gaussianB1 = imageArtifactConcatenationB.AddImageArtifact(std::move(gaussianArtifactB1));
    CuppingArtifact cuppingArtifactBsub;
    cuppingArtifactBsub.SetMinRadiodensityFactor(-300.0);
    BasicImageArtifact cuppingArtifactB(cuppingArtifactBsub);
    cuppingArtifactB.SetName("cupping(-300.0)");
    auto& cuppingB = imageArtifactConcatenationB.AddImageArtifact(std::move(cuppingArtifactB));


    PipelineGroup& pipelineGroupA = PipelineGroups.AddPipelineGroup(pipelineA, "PipelineGroup A");

    auto gaussianProperties = gaussian.GetProperties();
    auto& gaussianMeanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> gaussianMeanSpan (ArtifactVariantPointer(&gaussian),
                                           gaussianMeanProperty,
    { gaussianMeanProperty.Get(), gaussianMeanProperty.Get() + 750, 50.0 },
    "My Mean Property");
    pipelineGroupA.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(gaussianMeanSpan));


    PipelineGroup& pipelineGroupB = PipelineGroups.AddPipelineGroup(pipelineB, "PipelineGroup B");

    auto gaussianB1Properties = gaussianB1.GetProperties();
    auto& gaussianB1MeanProperty = gaussianB1Properties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> gaussianB1MeanSpan (ArtifactVariantPointer(&gaussianB1),
                                             gaussianB1MeanProperty,
    { gaussianB1MeanProperty.Get(),
                gaussianB1MeanProperty.Get() + 100.0F,
                50.0F },
    "Mean 1");
    pipelineGroupB.AddParameterSpan(ArtifactVariantPointer(&gaussianB1), std::move(gaussianB1MeanSpan));

    auto cuppingBProperties = cuppingB.GetProperties();
    auto& cuppingBCenterProperty = cuppingBProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan<FloatPoint> cuppingBCenterSpan (ArtifactVariantPointer(&cuppingB),
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

auto DebugSceneInitializer::operator()() -> void {
    auto cancellousBoneTissueType = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

    auto& thresholdFilter = dynamic_cast<ThresholdFilter&>(App_.GetThresholdFilter());
    thresholdFilter.ThresholdBetween(cancellousBoneTissueType.Radiodensity * 0.95,
                                     cancellousBoneTissueType.Radiodensity * 1.05);

    BasicStructure sphere(Sphere{});
    sphere.SetTissueType(cancellousBoneTissueType);
    sphere.SetTransformData({ 10.0F, 10.0F, 10.0F, 0.0F, 0.0F, 0.0F, 2.5F, 2.5F, 2.5F });

    CtDataTree.AddBasicStructure(std::move(sphere));

    static Range<float> const meanRange = { -25.0,  25.0, 5.0 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(meanRange.GetCenter());
    gaussianArtifact.SetStandardDeviation(20.0);

    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
    gaussianBasicArtifact.SetName("gaussian");
    auto& gaussian = concatenation.AddImageArtifact(std::move(gaussianBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");

    auto gaussianProperties = gaussian.GetProperties();

    auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> meanSpan {
            ArtifactVariantPointer(&gaussian),
            meanProperty,
            { meanRange.Min, meanRange.Max, meanRange.Step },
            "Mean Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));
}

DebugSingleSceneInitializer::DebugSingleSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto DebugSingleSceneInitializer::operator()() -> void {
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

    static Range<float> const saltAmountRange = { 0.0, 0.01, 0.001 };
    static Range<float> const pepperAmountRange = { 0.0, 0.01, 0.001 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    SaltPepperArtifact saltPepperArtifact {};
    saltPepperArtifact.SetSaltIntensity(1500.0F);
    saltPepperArtifact.SetPepperIntensity(-900.0F);
    saltPepperArtifact.SetSaltAmount(saltAmountRange.GetCenter());
    saltPepperArtifact.SetPepperAmount(pepperAmountRange.GetCenter());

    BasicImageArtifact saltPepperBasicArtifact { std::move(saltPepperArtifact) };
    saltPepperBasicArtifact.SetName("salt and pepper");
    auto& saltPepper = concatenation.AddImageArtifact(std::move(saltPepperBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Salt Pepper Pipelines");

    auto saltPepperProperties = saltPepper.GetProperties();

    auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
    ParameterSpan<float> saltAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            saltAmountProperty,
            { saltAmountRange.Min, saltAmountRange.Max, saltAmountRange.Step },
            "Salt Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(saltAmountSpan));

    auto& pepperAmountProperty = saltPepperProperties.GetPropertyByName<float>("Pepper Amount");
    ParameterSpan<float> pepperAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            pepperAmountProperty,
            { pepperAmountRange.Min, pepperAmountRange.Max, pepperAmountRange.Step },
            "Pepper Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(pepperAmountSpan));
}

SimpleSceneInitializer::SimpleSceneInitializer(App& app) :
        SceneInitializer(app) {}

auto SimpleSceneInitializer::operator()() -> void {
    auto cancellousBoneTissueType = BasicStructureDetails::GetTissueTypeByName("Cancellous Bone");

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

auto SimpleSceneInitializer::InitializeGaussian() noexcept -> void {
    static Range<float> const meanRange = { -70.0,  70.0, 10.0 };
    static Range<float> const sdRange   = {   0.0, 200.0, 25.0 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(meanRange.GetCenter());
    gaussianArtifact.SetStandardDeviation(sdRange.GetCenter());

    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
    gaussianBasicArtifact.SetName("gaussian");
    auto& gaussian = concatenation.AddImageArtifact(std::move(gaussianBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");

    auto gaussianProperties = gaussian.GetProperties();

    auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> meanSpan {
            ArtifactVariantPointer(&gaussian),
            meanProperty,
            { meanRange.Min, meanRange.Max, meanRange.Step },
            "Mean Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));

    auto& sdProperty = gaussianProperties.GetPropertyByName<float>("Standard Deviation");
    ParameterSpan<float> sdSpan {
            ArtifactVariantPointer(&gaussian),
            sdProperty,
            { sdRange.Min, sdRange.Max, sdRange.Step },
            "Standard Deviation Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(sdSpan));
}

auto SimpleSceneInitializer::InitializeCupping() noexcept -> void {
    static Range<float> const minRadiodensityFactorRange = { 0.0, 1.0, 0.1 };
    static Range<FloatPoint> const centerRange = { { -50.0, -50.0, -50.0 },
                                                   { 50.0, 50.0, 50.0 },
                                                   { 10.0, 10.0, 10.0 } };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    CuppingArtifact cuppingArtifact {};
    cuppingArtifact.SetMinRadiodensityFactor(minRadiodensityFactorRange.GetCenter());
    cuppingArtifact.SetCenter(centerRange.GetCenter());

    BasicImageArtifact cuppingBasicArtifact { std::move(cuppingArtifact) };
    cuppingBasicArtifact.SetName("cupping");
    auto& cupping = concatenation.AddImageArtifact(std::move(cuppingBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Cupping Pipelines");

    auto cuppingProperties = cupping.GetProperties();

    auto& minRadiodensityFactorProperty = cuppingProperties.GetPropertyByName<float>("Minimum Radiodensity Factor");
    ParameterSpan<float> minRadiodensityFactorSpan {
            ArtifactVariantPointer(&cupping),
            minRadiodensityFactorProperty,
            { minRadiodensityFactorRange.Min, minRadiodensityFactorRange.Max, minRadiodensityFactorRange.Step },
            "Minimum Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&cupping), std::move(minRadiodensityFactorSpan));

    auto& centerProperty = cuppingProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan<FloatPoint> centerSpan {
            ArtifactVariantPointer(&cupping),
            centerProperty,
            { centerRange.Min, centerRange.Max, centerRange.Step },
            "Center Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&cupping), std::move(centerSpan));
}

auto SimpleSceneInitializer::InitializeRing() noexcept -> void {
    static Range<float> const radiodensityFactorRange = { 0.25, 1.75, 0.125 };
    static Range<FloatPoint> const centerRange = { { -50.0, -50.0, -50.0 },
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
    auto& ring = concatenation.AddImageArtifact(std::move(ringBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Ring Pipelines");

    auto ringProperties = ring.GetProperties();

    auto& radiodensityFactorProperty = ringProperties.GetPropertyByName<float>("Radiodensity Factor");
    ParameterSpan<float> radiodensityFactorSpan {
            ArtifactVariantPointer(&ring),
            radiodensityFactorProperty,
            { radiodensityFactorRange.Min, radiodensityFactorRange.Max, radiodensityFactorRange.Step },
            "Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(radiodensityFactorSpan));

    auto& centerProperty = ringProperties.GetPropertyByName<FloatPoint>("Center");
    ParameterSpan<FloatPoint> centerSpan {
            ArtifactVariantPointer(&ring),
            centerProperty,
            { centerRange.Min, centerRange.Max, centerRange.Step },
            "Center Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&ring), std::move(centerSpan));
}

auto SimpleSceneInitializer::InitializeSaltPepper() noexcept -> void {
    static Range<float> const saltAmountRange = { 0.0, 0.01, 0.001 };
    static Range<float> const pepperAmountRange = { 0.0, 0.01, 0.001 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    SaltPepperArtifact saltPepperArtifact {};
    saltPepperArtifact.SetSaltIntensity(1500.0F);
    saltPepperArtifact.SetPepperIntensity(-900.0F);
    saltPepperArtifact.SetSaltAmount(saltAmountRange.GetCenter());
    saltPepperArtifact.SetPepperAmount(pepperAmountRange.GetCenter());

    BasicImageArtifact saltPepperBasicArtifact { std::move(saltPepperArtifact) };
    saltPepperBasicArtifact.SetName("salt and pepper");
    auto& saltPepper = concatenation.AddImageArtifact(std::move(saltPepperBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Salt Pepper Pipelines");

    auto saltPepperProperties = saltPepper.GetProperties();

    auto& saltAmountProperty = saltPepperProperties.GetPropertyByName<float>("Salt Amount");
    ParameterSpan<float> saltAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            saltAmountProperty,
            { saltAmountRange.Min, saltAmountRange.Max, saltAmountRange.Step },
            "Salt Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(saltAmountSpan));

    auto& pepperAmountProperty = saltPepperProperties.GetPropertyByName<float>("Pepper Amount");
    ParameterSpan<float> pepperAmountSpan {
            ArtifactVariantPointer(&saltPepper),
            pepperAmountProperty,
            { pepperAmountRange.Min, pepperAmountRange.Max, pepperAmountRange.Step },
            "Pepper Amount Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&saltPepper), std::move(pepperAmountSpan));
}

auto SimpleSceneInitializer::InitializeStairStep() noexcept -> void {
    static Range<float> const zSamplingRate = { 0.1, 1.0, 0.008 };

    auto& pipeline = Pipelines.AddPipeline();

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    StairStepArtifact stairStepArtifact {};
    stairStepArtifact.SetRelativeZAxisSamplingRate(zSamplingRate.GetCenter());

    BasicImageArtifact stairStepBasicArtifact { std::move(stairStepArtifact) };
    stairStepBasicArtifact.SetName("stair step");
    auto& stairStep = concatenation.AddImageArtifact(std::move(stairStepBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Stair Step Pipelines");

    auto stairStepProperties = stairStep.GetProperties();

    auto& zAxisSamplingRateProperty = stairStepProperties.GetPropertyByName<float>("z-Axis Sampling Rate");
    ParameterSpan<float> zAxisSamplingRateSpan {
            ArtifactVariantPointer(&stairStep),
            zAxisSamplingRateProperty,
            { zSamplingRate.Min, zSamplingRate.Max, zSamplingRate.Step },
            "z-Axis Sampling Rate Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&stairStep), std::move(zAxisSamplingRateSpan));
}

auto SimpleSceneInitializer::InitializeWindMill() noexcept -> void {
    static Range<float> const brightIntensityRange = { 0.0, 80.0, 10.0 };
    static Range<float> const angularWidth = { 1.0, 13.0, 4.0 };

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
    auto& windMill = concatenation.AddImageArtifact(std::move(windMillBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Wind Mill Pipelines");

    auto windMillProperties = windMill.GetProperties();

    auto& brightIntensityProperty = windMillProperties.GetPropertyByName<float>("Bright Intensity");
    ParameterSpan<float> brightIntensitySpan {
            ArtifactVariantPointer(&windMill),
            brightIntensityProperty,
            { brightIntensityRange.Min, brightIntensityRange.Max, brightIntensityRange.Step },
            "Bright Intensity Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&windMill), std::move(brightIntensitySpan));

    auto& brightAngularWidthProperty = windMillProperties.GetPropertyByName<float>("Bright Angular Width");
    ParameterSpan<float> brightAngularWidthSpan {
            ArtifactVariantPointer(&windMill),
            brightAngularWidthProperty,
            { angularWidth.Min, angularWidth.Max, angularWidth.Step },
            "Bright Angular Width Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&windMill), std::move(brightAngularWidthSpan));

    auto& darkAngularWidthProperty = windMillProperties.GetPropertyByName<float>("Dark Angular Width");
    ParameterSpan<float> darkAngularWidthSpan {
            ArtifactVariantPointer(&windMill),
            darkAngularWidthProperty,
            { angularWidth.Min, angularWidth.Max, angularWidth.Step },
            "Dark Angular Width Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&windMill), std::move(darkAngularWidthSpan));
}

auto SimpleSceneInitializer::InitializeMotion() noexcept -> void {
    static Range<float> const ctNumberFactorRange = { 0.5, 1.5, 0.01 };

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
    ParameterSpan<float> radiodensityFactorSpan {
            ArtifactVariantPointer(&motion),
            radiodensityFactorProperty,
            { ctNumberFactorRange.Min, ctNumberFactorRange.Max, ctNumberFactorRange.Step },
            "Radiodensity Factor Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&motion), std::move(radiodensityFactorSpan));
}



MethodologySceneInitializer::MethodologySceneInitializer(App& app) :
        SceneInitializer(app) {}

auto MethodologySceneInitializer::operator()() -> void {
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


    static Range<float> const meanRange = { -25.0,  25.0, 5.0 };

    auto& pipeline = Pipelines.Get(0);

    ImageArtifactConcatenation& concatenation = pipeline.GetImageArtifactConcatenation();

    GaussianArtifact gaussianArtifact {};
    gaussianArtifact.SetMean(meanRange.GetCenter());
    gaussianArtifact.SetStandardDeviation(20.0);

    BasicImageArtifact gaussianBasicArtifact { std::move(gaussianArtifact) };
    gaussianBasicArtifact.SetName("gaussian");
    auto& gaussian = concatenation.AddImageArtifact(std::move(gaussianBasicArtifact));

    PipelineGroup& pipelineGroup = PipelineGroups.AddPipelineGroup(pipeline, "Gaussian Pipelines");

    auto gaussianProperties = gaussian.GetProperties();

    auto& meanProperty = gaussianProperties.GetPropertyByName<float>("Mean");
    ParameterSpan<float> meanSpan {
            ArtifactVariantPointer(&gaussian),
            meanProperty,
            { meanRange.Min, meanRange.Max, meanRange.Step },
            "Mean Span"
    };
    pipelineGroup.AddParameterSpan(ArtifactVariantPointer(&gaussian), std::move(meanSpan));
}
