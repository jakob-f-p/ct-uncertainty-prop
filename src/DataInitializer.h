#pragma once

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
        SPHERE,
        SIMPLE_SCENE
    };

    auto
    operator()(Config config) -> void;

private:
    auto
    InitializeSpherical() noexcept -> void;

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
    };

protected:
    App& App_;
    CtStructureTree& CtDataTree;
    PipelineList& Pipelines;
    PipelineGroupList& PipelineGroups;
};


class DefaultSceneInitializer : public SceneInitializer {
public:
    explicit DefaultSceneInitializer(App& app);

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
