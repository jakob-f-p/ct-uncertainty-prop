#pragma once

#include <QMainWindow>

#include <vtkNew.h>

class CtDataSource;
class CtStructureTree;
class PipelineList;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(CtStructureTree& ctStructureTree, CtDataSource& dataSource, PipelineList& pipelineList);

private:
    CtDataSource& DataSource;
};
