#pragma once

#include "ArtifactVariantPointer.h"
#include "ObjectProperty.h"
#include "PipelineParameterSpaceState.h"

class Pipeline;

template<typename T>
class ParameterSpan {
public:
    struct NumberDetails {
        T Min;
        T Max;
        T Step;

        [[nodiscard]] auto
        GetNumberOfPipelines() const noexcept -> uint32_t;

        [[nodiscard]] auto
        operator== (NumberDetails const& other) const noexcept -> bool;

        [[nodiscard]] auto
        ToString() const noexcept -> std::string;
    };

    ParameterSpan(ArtifactVariantPointer artifactPointer,
                  ObjectProperty<T> objectProperty,
                  NumberDetails numbers,
                  std::string name = "");
    ParameterSpan(ParameterSpan&& other) noexcept = default;
    auto operator= (ParameterSpan&& other) noexcept -> ParameterSpan& = default;
    ParameterSpan(ParameterSpan const& other) = delete;
    auto operator= (ParameterSpan const& other) -> ParameterSpan& = delete;
    ~ParameterSpan() = default;

    [[nodiscard]] auto
    GetName() const noexcept -> std::string { return Name; }

    auto
    SetName(std::string name) noexcept -> void { Name = std::move(name); }

    [[nodiscard]] auto
    GetPropertyName() const noexcept -> std::string { return Property.GetName(); }

    [[nodiscard]] auto
    GetNumberOfPipelines() const noexcept -> uint32_t { return Numbers.GetNumberOfPipelines(); }

    [[nodiscard]] auto
    GetArtifact() const noexcept -> ArtifactVariantPointer { return ArtifactPointer; }

    [[nodiscard]] auto
    GetNumbers() const noexcept -> NumberDetails { return Numbers; }

    [[nodiscard]] auto
    operator== (ParameterSpan const& other) const noexcept -> bool;

private:
    template<typename U>
    friend class SpanState;
    template<typename U>
    friend class SpanStateSourceIterator;

    ArtifactVariantPointer ArtifactPointer;
    std::string Name;
    ObjectProperty<T> Property;
    NumberDetails Numbers;
};



struct PipelineParameterSpan {

    template<typename... Args>
    PipelineParameterSpan(Args&&... args) : SpanVariant(std::forward<Args>(args)...) {};

    [[nodiscard]] auto
    GetArtifact() const noexcept -> ArtifactVariantPointer;

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetPropertyName() const noexcept -> std::string;

    auto
    SetName(const std::string& name) noexcept -> void;

    [[nodiscard]] auto
    GetNumberOfPipelines() const noexcept -> uint32_t;

    [[nodiscard]] auto
    States() -> std::vector<ParameterSpanState>;

    struct Range {
        float Min;
        float Max;
    };

    [[nodiscard]] auto
    GetRange() const noexcept -> Range;

    [[nodiscard]] auto
    operator== (PipelineParameterSpan const& other) const noexcept -> bool;

    template<typename Type>
    [[nodiscard]] auto
    operator== (Type const& other) const noexcept -> bool {
        return std::holds_alternative<Type>(SpanVariant)
                       && &std::get<Type>(SpanVariant) == &other;
    };

private:
    friend class ObjectPropertyGroup;
    friend class ParameterSpanState;
    friend struct ParameterSpanStateSourceIterator;

    using ParameterSpanVariant = std::variant<ParameterSpan<float>, ParameterSpan<FloatPoint>>;
    ParameterSpanVariant SpanVariant;
};
