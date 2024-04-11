#pragma once

#include <QMainWindow>

class CtStructureTree;
class CtStructureTreeModel;
class CtDataSource;
class CtStructureDialog;
class ModelingRenderWidget;

struct BasicStructureData;
struct CombinedStructureData;

class QItemSelectionModel;
class QPushButton;
class QTreeView;

class ModelingWidget : public QMainWindow {
public:
    explicit ModelingWidget(QWidget* parent = nullptr);
    ~ModelingWidget() override = default;

private:
    void ConnectButtons();

    void DisableButtons();

    void OpenBasicAndCombinedStructureCreateDialog(
            const std::function<const void(const BasicStructureData&, const CombinedStructureData&)>& onAccepted);

    ModelingRenderWidget* RenderWidget;

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
};