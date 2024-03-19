#pragma once

#include "CtStructureEditDialog.h"
#include "CtDataCsgTreeModel.h"
#include "CtDataSource.h"

#include <QItemSelectionModel>
#include <QMainWindow>
#include <QPushButton>
#include <QTreeView>

class ModelingWidget : public QMainWindow {
Q_OBJECT

public:
    ModelingWidget();

public slots:
    void OpenDialog(const std::function<const void()>& onAccepted);


private:
    void SetUpRenderingWidgetForShowingImplicitData();

    void SetUpDockWidgetForImplicitCsgTreeModeling();

    void ConnectButtons();

    void DisableButtons();

    QPushButton* AddStructureButton;
    QPushButton* CombineWithStructureButton;
    QPushButton* RefineWithStructureButton;

    QPushButton* RemoveStructureButton;
    CtDataCsgTreeModel* TreeModel;
    QTreeView* TreeView;
    QItemSelectionModel* SelectionModel;

    CtStructureEditDialog* CtStructureCreateDialog;
    CtDataSource* DataSource;
    CtDataCsgTree* DataTree;
};