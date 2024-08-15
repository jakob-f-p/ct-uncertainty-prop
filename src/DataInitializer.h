#pragma once

#include "Utils/LinearAlgebraTypes.h"

#include <cstdint>

class App;
class CtStructureTree;
class PipelineGroupList;
class PipelineList;


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
    };

    auto
    operator()(Config config) -> void;

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
    operator()() -> void;
};


class DebugSceneInitializer : public SceneInitializer {
public:
    explicit DebugSceneInitializer(App& app);

    auto
    operator()() -> void;
};

class DebugSingleSceneInitializer : public SceneInitializer {
public:
    explicit DebugSingleSceneInitializer(App& app);

    auto
    operator()() -> void;
};


class SimpleSceneInitializer : public SceneInitializer {
public:
    explicit SimpleSceneInitializer(App& app);

    auto
    operator()() -> void;

private:
    auto
    InitializeGaussian() noexcept -> void;

    auto
    InitializeCupping() noexcept -> void;

    auto
    InitializeRing() noexcept -> void;

    auto
    InitializeSaltPepper() noexcept -> void;

    auto
    InitializeStairStep() noexcept -> void;

    auto
    InitializeWindMill() noexcept -> void;

    auto
    InitializeMotion() noexcept -> void;
};



class MethodologyAcquisitionSceneInitializer : public SceneInitializer {
public:
    explicit MethodologyAcquisitionSceneInitializer(App& app);

    auto
    operator()() -> void;
};


class MethodologyArtifactsSceneInitializer : public SceneInitializer {
public:
    explicit MethodologyArtifactsSceneInitializer(App& app);

    auto
    operator()() -> void;
};

class MethodologySegmentationSceneInitializer : public SceneInitializer {
public:
    explicit MethodologySegmentationSceneInitializer(App& app);

    auto
    operator()() -> void;
};

class MethodologyAnalysisSceneInitializer : public SceneInitializer {
public:
    explicit MethodologyAnalysisSceneInitializer(App& app);

    auto
    operator()() -> void;
};
