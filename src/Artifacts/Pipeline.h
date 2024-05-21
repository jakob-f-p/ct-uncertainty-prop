#pragma once

#include <cstdint>
#include <memory>
#include <string>

class CtDataSource;
class CtStructureTree;
class ImageArtifactConcatenation;
class StructureArtifactList;
class TreeStructureArtifactListCollection;

struct CtStructureTreeEvent;

class vtkImageAlgorithm;

class Pipeline {
public:
    explicit Pipeline(CtStructureTree const& structureTree, CtDataSource& dataSource);
    ~Pipeline();

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetStructureArtifactListCollection(uint16_t structureIdx) const -> StructureArtifactList&;

    [[nodiscard]] auto
    GetImageArtifactConcatenation() const -> ImageArtifactConcatenation&;

    [[nodiscard]] auto
    GetImageAlgorithm() const -> vtkImageAlgorithm&;

    auto ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) const -> void;

    auto operator==(const Pipeline& other) const noexcept -> bool;

    std::string Name;
    CtDataSource& DataSource;
    std::unique_ptr<TreeStructureArtifactListCollection> TreeStructureArtifacts;
    std::unique_ptr<ImageArtifactConcatenation> ImageArtifactConcat;
};
