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
    explicit MainWindow(CtStructureTree& ctStructureTree,
                        CtDataSource& dataSource,
                        ThresholdFilter& thresholdFilter,
                        PipelineList& pipelineList,
                        PipelineGroupList& pipelineGroups);

private:
    CtDataSource& DataSource;
};
