#pragma once

#include "../Utils/RenderWidget.h"

#include <QMainWindow>

class BasicStructureData;
class CombinedStructureData;
class CtDataSource;
class CtStructureTreeModel;
class CtStructureDialog;
class CtStructureTree;
class CtStructureView;
class RenderWidget;

class QItemSelection;
class QItemSelectionModel;
class QPushButton;

class ModelingWidget : public QMainWindow {
public:
    explicit ModelingWidget(CtStructureTree& ctStructureTree,
                            CtDataSource& dataSource,
                            QWidget* parent = nullptr);

private:
    void ConnectButtons();

    void DisableButtons();

    void OpenBasicAndCombinedStructureCreateDialog(
            const std::function<const void(BasicStructureData const&, CombinedStructureData const&)>& onAccepted);

    void UpdateButtonStates(QItemSelection const& selected, QItemSelection const&);

    RenderWidget* const RenderingWidget;

    QPushButton* const AddStructureButton;
    QPushButton* const CombineWithStructureButton;
    QPushButton* const RefineWithStructureButton;
    QPushButton* const RemoveStructureButton;
    std::array<QPushButton* const, 4> const CtStructureButtons;

    CtStructureView* const TreeView;
    CtStructureTreeModel* const TreeModel;
    QItemSelectionModel* const SelectionModel;

    CtStructureDialog* CtStructureCreateDialog;
};
