#pragma once

#include "StructureArtifact.h"
#include "../../Types.h"

#include <vtkNew.h>

#include <memory>
#include <vector>

class CtStructureTree;
class StructureArtifactsFilter;
class vtkImageAlgorithm;

class StructureArtifactList {
public:
    using BasicStructureIdProvider = std::function<std::vector<StructureId>(StructureArtifactList const&)>;
    using TissueValueProvider = std::function<StructureArtifact::StructureEvaluator(StructureArtifactList const&)>;
    using StructureEvaluatorProvider = std::function<StructureArtifact::StructureEvaluator(StructureArtifactList const&)>;

    StructureArtifactList(BasicStructureIdProvider&& basicStructureIdProvider,
                          TissueValueProvider&& tissueValueProvider,
                          StructureEvaluatorProvider&& structureEvaluatorProvider):
            BasicStructureIdProv(std::move(basicStructureIdProvider)),
            TissueValueProv(std::move(tissueValueProvider)),
            StructureEvaluatorProv(std::move(structureEvaluatorProvider)) {};
    StructureArtifactList(StructureArtifactList const&) = default;
    StructureArtifactList(StructureArtifactList&&) = default;
    auto operator= (StructureArtifactList const&) -> StructureArtifactList& = default;
    auto operator= (StructureArtifactList&&) -> StructureArtifactList& = default;
    ~StructureArtifactList();

    [[nodiscard]] auto
    operator== (StructureArtifactList const& other) const noexcept -> bool { return this == &other; }

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

    [[nodiscard]] auto
    Get(int idx) -> StructureArtifact&;

    [[nodiscard]] auto
    GetNumberOfArtifacts() const noexcept -> uidx_t;

    auto
    AddStructureArtifact(StructureArtifact&& structureArtifact, int insertionIdx = -1) -> void;

    auto
    RemoveStructureArtifact(StructureArtifact const& structureArtifact) -> void;

    auto
    MoveStructureArtifact(StructureArtifact const& artifact, int newIdx) -> void;

    struct Result {
        using SubType = StructureArtifact::SubType;

        [[nodiscard]] auto
        operator[](SubType subType) noexcept -> float& { return ArtifactValues[static_cast<int>(subType)]; }

        [[nodiscard]] constexpr auto
        operator[](SubType subType) const noexcept -> float const& { return ArtifactValues[static_cast<int>(subType)]; }

        [[nodiscard]] constexpr auto
        operator+(Result const& other) const noexcept -> Result {
            Result result;
            for (uidx_t i = 0; i < ArtifactValues.size(); i++)
                result.ArtifactValues[i] = ArtifactValues[i] + other.ArtifactValues[i];

            return result;
        }

        [[nodiscard]] auto
        GetSum() const noexcept -> float { return std::reduce(ArtifactValues.cbegin(), ArtifactValues.cend()); }

    private:
        std::array<float, StructureArtifactDetails::GetNumberOfSubTypeValues()> ArtifactValues {};
    };

    auto
    UpdateBasicStructureIds() noexcept -> void { BasicStructureIds = BasicStructureIdProv(*this); }

    [[nodiscard]] auto
    ArtifactValues(DoublePoint point, StructureId structureId) const noexcept -> Result {
        bool pointOccupiedByStructure = std::any_of(BasicStructureIds.cbegin(), BasicStructureIds.cend(),
                                                    [&](StructureId id) { return id == structureId; });

        Result result {};

//        for (const auto& artifact : Artifacts)
//            result[artifact.GetSubType()] += artifact.EvaluateAtPosition(point, pointOccupiedByStructure);

        return result;
    }

    template<typename Calculate>
    auto
    ArtifactValue(Calculate calculate) const noexcept -> void {
        auto const basicStructureIds = BasicStructureIdProv(*this);
        auto tissueValueEvaluator = TissueValueProv(*this);
        auto functionValueEvaluator = StructureEvaluatorProv(*this);

        for (const auto& artifact : Artifacts)
            calculate(artifact.GetSubType(),
                      basicStructureIds,
                      [&](DoublePoint point, bool pointOccupiedByStructure) {
                              return artifact.EvaluateAtPosition(point, pointOccupiedByStructure,
                                                                 tissueValueEvaluator(point),
                                                                 functionValueEvaluator);
                      }
            );
    }

private:
    friend class TreeStructureArtifactListCollection;

    std::vector<StructureArtifact> Artifacts;

    // Basic structure ids associated with this artifact list
    // If artifact refers to basic structure, then this vector contains one element. For composite structures it
    // contains multiple IDs.
    BasicStructureIdProvider BasicStructureIdProv;
    std::vector<StructureId> BasicStructureIds;

    TissueValueProvider TissueValueProv;

    StructureEvaluatorProvider StructureEvaluatorProv;
};


class TreeStructureArtifactListCollection {
public:
    explicit TreeStructureArtifactListCollection(CtStructureTree const& ctStructureTree);
    ~TreeStructureArtifactListCollection();

    [[nodiscard]] auto
    GetMTime() -> vtkMTimeType;

    [[nodiscard]] auto
    GetForCtStructureIdx(uidx_t structureIdx) -> StructureArtifactList&;

    auto
    AddStructureArtifactList(uidx_t insertionIdx) -> void;

    auto
    RemoveStructureArtifactList(uidx_t removeIdx) -> void;

    [[nodiscard]] auto
    ArtifactValues(DoublePoint point, StructureId structureId) const noexcept -> StructureArtifactList::Result {
        return std::transform_reduce(
                ArtifactLists.cbegin(), ArtifactLists.cend(), StructureArtifactList::Result{}, std::plus{},
                [&](auto const& artifactList) { return artifactList.ArtifactValues(point, structureId); }
        );
    }

    template<typename Calculate>
    auto
    ArtifactValue(Calculate&& calculate) const noexcept -> void {
        for (const auto& artifactList : ArtifactLists)
            artifactList.ArtifactValue(std::forward<Calculate>(calculate));
    }

    [[nodiscard]] auto
    GetFilter() const -> vtkImageAlgorithm&;

private:
    CtStructureTree const& StructureTree;

    std::vector<StructureArtifactList> ArtifactLists;

    vtkNew<StructureArtifactsFilter> Filter;
};
