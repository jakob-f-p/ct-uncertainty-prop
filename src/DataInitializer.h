#pragma once

#include "Utils/LinearAlgebraTypes.h"

#include <cstdint>
#include <memory>

class App;
class BasicImageArtifact;
class CtStructureTree;
class ImageArtifact;
class PipelineGroup;
class PipelineGroupList;
class PipelineList;

class vtkImageData;

class DataInitializer {
public:
    explicit DataInitializer(App& app);

    enum struct Config : uint8_t {
        DEFAULT,
        DEBUG,
        DEBUG_SINGLE,
        SIMPLE_SCENE,
        METHODOLOGY_ACQUISITION,
        METHODOLOGY_ARTIFACTS,
        METHODOLOGY_SEGMENTATION,
        METHODOLOGY_ANALYSIS,
        SCENARIO_IMPLICIT,
        SCENARIO_IMPORTED,
        WORKFLOW_FIGURE
    };

    auto
    operator()(Config config) const -> void;

private:
    App& App_;
    CtStructureTree& CtDataTree;
    PipelineList& Pipelines;
    PipelineGroupList& PipelineGroups;
};


class SceneInitializer {
public:
    explicit SceneInitializer(App& app);

    template<typename T>
    struct Range {
        T Min;
        T Max;
        T Step;

        [[nodiscard]] auto
        GetCenter() const noexcept -> T { return (Max + Min) / 2; }
    };

protected:
    App& App_;
    CtStructureTree& CtDataTree;
    PipelineList& Pipelines;
    PipelineGroupList& PipelineGroups;
};

template<>
auto inline SceneInitializer::Range<FloatPoint>::GetCenter() const noexcept -> FloatPoint {
    FloatPoint result {};
    for (int i = 0; i < 3; i++)
        result[i] = (Max[i] + Min[i]) / 2;

    return result;
}


class DefaultSceneInitializer : public SceneInitializer {
public:
    explicit DefaultSceneInitializer(App& app);

    auto
    operator()() const -> void;
};


class DebugSceneInitializer : public SceneInitializer {
public:
    explicit DebugSceneInitializer(App& app);

    auto
    operator()() const -> void;
};

class DebugSingleSceneInitializer : public SceneInitializer {
public:
    explicit DebugSingleSceneInitializer(App& app);

    auto
    operator()() const -> void;
};


class SimpleSceneInitializer : public SceneInitializer {
public:
    explicit SimpleSceneInitializer(App& app);

    auto
    operator()() const -> void;

private:
    auto
    InitializeGaussian() const noexcept -> void;

    auto
    InitializeCupping() const noexcept -> void;

    auto
    InitializeRing() const noexcept -> void;

    auto
    InitializeSaltPepper() const noexcept -> void;

    auto
    InitializeStairStep() const noexcept -> void;

    auto
    InitializeWindMill() const noexcept -> void;

    auto
    InitializeMotion() const noexcept -> void;
};



class MethodologyAcquisitionSceneInitializer : public SceneInitializer {
public:
    explicit MethodologyAcquisitionSceneInitializer(App& app);

    auto
    operator()() const -> void;
};


class MethodologyArtifactsSceneInitializer : public SceneInitializer {
public:
    explicit MethodologyArtifactsSceneInitializer(App& app);

    auto
    operator()() const -> void;
};

class MethodologySegmentationSceneInitializer : public SceneInitializer {
public:
    explicit MethodologySegmentationSceneInitializer(App& app);

    auto
    operator()() const -> void;
};

class MethodologyAnalysisSceneInitializer : public SceneInitializer {
public:
    explicit MethodologyAnalysisSceneInitializer(App& app);

    auto
    operator()() const -> void;
};

class ScenarioImplicitInitializer : public SceneInitializer {
public:
    explicit ScenarioImplicitInitializer(App& app);

    auto
    operator()() const -> void;

private:
    auto
    InitializeSaltPepper() const noexcept -> void;

    auto
    InitializeGaussian() const noexcept -> void;

    auto
    InitializeCupping() const noexcept -> void;

    auto
    InitializeRing() const noexcept -> void;

    auto
    InitializeMetal() const noexcept -> void;

    auto
    InitializeWindmill() const noexcept -> void;

    auto
    InitializeMotion() const noexcept -> void;
};

class ScenarioImportedInitializer : public SceneInitializer {
public:
    explicit ScenarioImportedInitializer(App& app);

    auto
    operator()() const -> void;

private:
    struct BasicImageArtifacts {
        std::unique_ptr<BasicImageArtifact> SaltPepper;
        std::unique_ptr<BasicImageArtifact> Gaussian;
        std::unique_ptr<BasicImageArtifact> Cupping;
        std::unique_ptr<BasicImageArtifact> Ring;
    };

    struct ImageArtifacts {
        ImageArtifact& SaltPepper;
        ImageArtifact& Gaussian;
        ImageArtifact& Cupping;
        ImageArtifact& Ring;
    };

    [[nodiscard]] static auto
    CreateBasicImageArtifacts() -> BasicImageArtifacts;

    static auto
    AddParameterSpans(PipelineGroup& pipelineGroup, ImageArtifacts& artifacts) -> void;

    static auto
    CalculateOtsuThreshold(vtkImageData& image) -> double;

    constexpr static Range<float> GaussianSdRange { 0.0, 30.0, 10.0 };
    constexpr static Range<float> CuppingMinRdFactorRange { 0.7, 1.0, 0.1 };
    constexpr static Range<float> RingInnerRadiusRange { 0.0, 15.0, 5.0 };
    constexpr static Range<float> SaltAmountRange { 0.00, 0.01, 0.01 };
    constexpr static Range<float> PepperAmountRange { 0.00, 0.01, 0.01 };
};


class WorkflowFigureInitializer : public SceneInitializer {
public:
    explicit WorkflowFigureInitializer(App& app);

    auto
    operator()() const -> void;

private:
    auto
    InitializeSaltPepper() const noexcept -> void;

    auto
    InitializeRing() const noexcept -> void;
};