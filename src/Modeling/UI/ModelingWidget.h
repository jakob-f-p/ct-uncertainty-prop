#pragma once

#include "../BasicStructure.h"
#include "../CombinedStructure.h"

#include <QMainWindow>

class CtStructureTree;
class CtStructureTreeModel;
class CtDataSource;
class CtStructureDialog;
class ModelingRenderWidget;

class QItemSelection;
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
            const std::function<const void(const BasicStructureDataVariant&, const CombinedStructureData&)>& onAccepted);

    void UpdateButtonStates(const QItemSelection& selected, const QItemSelection&);

    ModelingRenderWidget* const RenderWidget;

    QPushButton* const ResetCameraButton;
    QPushButton* const AddStructureButton;
    QPushButton* const CombineWithStructureButton;
    QPushButton* const RefineWithStructureButton;
    QPushButton* const RemoveStructureButton;
    const std::array<QPushButton* const, 4> CtStructureButtons;

    CtStructureTreeModel* const TreeModel;
    QTreeView* const TreeView;
    QItemSelectionModel* SelectionModel;

    CtStructureDialog* CtStructureCreateDialog;
};