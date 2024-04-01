#pragma once

#include <QItemSelectionModel>
#include <QMainWindow>
#include <QPushButton>
#include <QTreeView>
#include <QVTKInteractor.h>

#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>

class CtStructureTree;
class CtStructureTreeModel;
class CtDataSource;
class CtStructureDialog;

class ModelingWidget : public QMainWindow {
Q_OBJECT

public:
    ModelingWidget();
    ~ModelingWidget() override;

private:
    void SetUpCentralWidgetForRendering();

    void SetUpDockWidgetForImplicitCtDataModeling();

    void ConnectButtons();

    void DisableButtons();

    void OpenCreateDialog(const std::function<const void()>& onAccepted);

    QPushButton* ResetCameraButton;
    QPushButton* AddStructureButton;
    QPushButton* CombineWithStructureButton;
    QPushButton* RefineWithStructureButton;
    QPushButton* RemoveStructureButton;
    std::array<QPushButton*, 4> CtStructureButtons;

    CtStructureTreeModel* TreeModel;
    QTreeView* TreeView;
    QItemSelectionModel* SelectionModel;

    CtStructureDialog* CtStructureCreateDialog;
    CtDataSource* DataSource;
    CtStructureTree* DataTree;

    vtkOrientationMarkerWidget* OrientationMarkerWidget;
    vtkOpenGLRenderer* Renderer;
    QVTKInteractor* RenderWindowInteractor;
    std::array<double, 3> InitialCameraPosition;
    vtkCamera* InitialCamera;
};