#pragma once

#include <QMainWindow>

#include <vtkNew.h>

class CtDataSource;
class CtStructureTree;
class PipelineList;
class PipelineGroupList;
class ThresholdFilter;

class MainWindow : public QMainWindow {
public:
    enum struct Mode : uint8_t { NORMAL, PRESENTATION };

    explicit MainWindow(CtStructureTree& ctStructureTree,
                        ThresholdFilter& thresholdFilter,
                        PipelineList& pipelineList,
                        PipelineGroupList& pipelineGroups,
                        Mode mode = Mode::NORMAL);
};
