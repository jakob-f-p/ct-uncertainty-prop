#pragma once

#include <QMainWindow>

class ArtifactsWidget;
class CtDataSource;
class CtStructureTree;
class DataGenerationWidget;
class ModelingWidget;
class PipelineList;
class PipelineGroupList;
class SegmentationWidget;
class ThresholdFilter;

class MainWindow : public QMainWindow {
public:
    enum struct Mode : uint8_t { NORMAL, PRESENTATION };

    explicit MainWindow(CtStructureTree& ctStructureTree,
                        ThresholdFilter& thresholdFilter,
                        PipelineList& pipelineList,
                        PipelineGroupList& pipelineGroups,
                        Mode mode = Mode::NORMAL);

    auto
    UpdateDataSource(CtDataSource& dataSource) const noexcept -> void;

protected:
    auto
    keyPressEvent(QKeyEvent* event) -> void override;

private:
    ModelingWidget* ModelingWidget_;
    ArtifactsWidget* ArtifactsWidget_;
    SegmentationWidget* SegmentationWidget_;
    DataGenerationWidget* DataGenerationWidget_;
};
