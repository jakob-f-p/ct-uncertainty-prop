#pragma once

#include <QMainWindow>

#include <vtkNew.h>

class CtDataSource;
class CtStructureTree;
class PipelineList;
class PipelineGroupList;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(CtStructureTree& ctStructureTree,
                        CtDataSource& dataSource,
                        PipelineList& pipelineList,
                        PipelineGroupList& pipelineGroups);

private:
    CtDataSource& DataSource;
};
