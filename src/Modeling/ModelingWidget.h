#pragma once

#include "CtStructureEditDialog.h"
#include "CtDataCsgTreeModel.h"

#include <QItemSelectionModel>
#include <QMainWindow>
#include <QPushButton>
#include <QTreeView>

class ModelingWidget : public QMainWindow {
Q_OBJECT

public:
    ModelingWidget();

private:
    void SetUpRenderingWidgetForShowingImplicitData();

    void SetUpDockWidgetForImplicitCsgTreeModeling();

    QPushButton* AddStructureButton;
    QPushButton* CombineWithStructureButton;
    QPushButton* RefineWithStructureButton;
    QPushButton* RemoveStructureButton;

    CtDataCsgTreeModel* TreeModel;
    QTreeView* TreeView;
    QItemSelectionModel* SelectionModel;
    CtStructureEditDialog* CtStructureCreateDialog;
};