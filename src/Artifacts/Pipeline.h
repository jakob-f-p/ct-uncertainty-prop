#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <vtkType.h>

class CtDataSource;
class CtStructureTree;
class ImageArtifactConcatenation;
class StructureArtifactList;
class TreeStructureArtifactListCollection;

struct CtStructureTreeEvent;

class vtkImageAlgorithm;

class Pipeline {
public:
    explicit Pipeline(CtStructureTree& structureTree, CtDataSource& dataSource, std::string name = "");
    ~Pipeline();

    [[nodiscard]] auto
    GetName() const noexcept -> std::string;

    [[nodiscard]] auto
    GetMTime() const noexcept -> vtkMTimeType;

    [[nodiscard]] auto
    GetCtStructureTree() const noexcept -> CtStructureTree&;

    [[nodiscard]] auto
    GetStructureArtifactListCollection() const -> TreeStructureArtifactListCollection&;

    [[nodiscard]] auto
    GetStructureArtifactList(uint16_t structureIdx) const -> StructureArtifactList&;

    [[nodiscard]] auto
    GetImageArtifactConcatenation() const -> ImageArtifactConcatenation&;

    [[nodiscard]] auto
    GetImageAlgorithm() const -> vtkImageAlgorithm&;

    auto ProcessCtStructureTreeEvent(const CtStructureTreeEvent& event) const -> void;

    auto operator==(const Pipeline& other) const noexcept -> bool;

private:
    std::string Name;
    CtStructureTree& StructureTree;
    CtDataSource& DataSource;
    std::unique_ptr<TreeStructureArtifactListCollection> TreeStructureArtifacts;
    std::unique_ptr<ImageArtifactConcatenation> ImageArtifactConcat;

    static uint16_t PipelineId;
};
