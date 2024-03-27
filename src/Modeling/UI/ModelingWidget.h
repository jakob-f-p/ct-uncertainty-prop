#pragma once

#include <QItemSelectionModel>
#include <QMainWindow>
#include <QPushButton>
#include <QTreeView>
#include <QVTKInteractor.h>

#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>

class CtDataCsgTree;
class CtDataCsgTreeModel;
class CtDataSource;
class CtStructureEditDialog;

class ModelingWidget : public QMainWindow {
Q_OBJECT

public:
    ModelingWidget();
    ~ModelingWidget() override;

public slots:
    void OpenCreateDialog(const std::function<const void()>& onAccepted);

private:
    void SetUpCentralWidgetForRendering();

    void SetUpDockWidgetForImplicitCsgTreeModeling();

    void ConnectButtons();

    void DisableButtons();

    QPushButton* ResetCameraButton;
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

    vtkOrientationMarkerWidget* OrientationMarkerWidget;
    vtkOpenGLRenderer* Renderer;
    QVTKInteractor* RenderWindowInteractor;
    std::array<double, 3> InitialCameraPosition;
    vtkCamera* InitialCamera;
};