#pragma once

#include "StructureArtifact.h"
#include "../../Utils/LinearAlgebraTypes.h"

#include <vtkNew.h>

#include <memory>
#include <ranges>
#include <vector>

class CtStructureTree;
class CtStructureVariant;
class StructureArtifactsFilter;
class vtkImageAlgorithm;

class StructureArtifactList {
public:
    using BeforeRemoveArtifactCallback = std::function<void(StructureArtifact&)>;
    using StructureProvider = std::function<CtStructureVariant const&(StructureArtifactList const&)>;

    StructureArtifactList(BeforeRemoveArtifactCallback removeCallback,
                          StructureProvider&& structureEvaluatorProvider):
            BeforeRemoveCallback(std::move(removeCallback)),
            StructureProv(std::move(structureEvaluatorProvider)) {};
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
    Contains(StructureArtifact const& structureArtifact) const noexcept -> bool;

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

    template<StructureArtifact::SubType ArtifactType>
    [[nodiscard]] auto
    GetStructureArtifacts() const -> std::vector<std::reference_wrapper<StructureArtifact const>> {
        auto filteredArtifacts = Artifacts | std::ranges::views::filter(
                [](StructureArtifact const& artifact) { return artifact.GetSubType() == ArtifactType; });

        std::vector<std::reference_wrapper<StructureArtifact const>> result { filteredArtifacts.begin(), filteredArtifacts.end() };

        return result;
    }

private:
    friend class TreeStructureArtifactListCollection;
    friend class CtStructureArtifactsModel;

    std::vector<StructureArtifact> Artifacts;

    BeforeRemoveArtifactCallback BeforeRemoveCallback;

    StructureProvider StructureProv;
};


class TreeStructureArtifactListCollection {
public:
    using BeforeRemoveArtifactCallback = std::function<void(StructureArtifact&)>;

    explicit TreeStructureArtifactListCollection(
            CtStructureTree const& ctStructureTree,
            BeforeRemoveArtifactCallback&& removeCallback = [](StructureArtifact&) {});
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
    GetFilter() const -> vtkImageAlgorithm&;

    [[nodiscard]] auto
    GetStructureArtifactList(StructureArtifact const& structureArtifact) const -> StructureArtifactList const&;

    constexpr static auto const StructureHash = [](CtStructureVariant const& structure) {
        return std::hash<CtStructureVariant const*>{}(&structure);
    };
    constexpr static auto const StructureEqual = [](CtStructureVariant const& left, CtStructureVariant const& right) {
        return &left == &right;
    };
    using StructureArtifactsMap = std::unordered_map<std::reference_wrapper<CtStructureVariant const>,
                                                    std::vector<std::reference_wrapper<StructureArtifact const>>,
                                                    decltype(StructureHash), decltype(StructureEqual)>;
    template<StructureArtifact::SubType ArtifactType>
    [[nodiscard]] auto
    GetStructureArtifacts() const -> StructureArtifactsMap {
        StructureArtifactsMap structureArtifactsMap;

        for (auto const& artifactList : ArtifactLists) {
            auto const& structure = artifactList.StructureProv(artifactList);
            auto structureArtifacts = artifactList.GetStructureArtifacts<ArtifactType>();
            structureArtifactsMap.emplace(structure, structureArtifacts);
        }

        return structureArtifactsMap;
    }

    [[nodiscard]] auto
    GetIdx(StructureArtifactList const& structureArtifactList) const -> uidx_t;

private:
    CtStructureTree const& StructureTree;
    BeforeRemoveArtifactCallback BeforeRemoveCallback;

    std::vector<StructureArtifactList> ArtifactLists;

    vtkNew<StructureArtifactsFilter> Filter;
};
