#pragma once

#include <QDialog>

class ArtifactsDialog;
class StructureArtifactList;
class StructureArtifactsModel;

class QItemSelection;
class QItemSelectionModel;
class QListView;

class StructureArtifactsWidgetDialog : public QDialog {
public:
    explicit StructureArtifactsWidgetDialog(StructureArtifactList& structureWrapper,
                                            std::string& title,
                                            QWidget* parent = nullptr);

protected Q_SLOTS:
    void AddArtifact();
    void RemoveArtifact();
    void MoveUp();
    void MoveDown();
    void UpdateButtonStatesOnSelectionChange(const QItemSelection& selected, const QItemSelection& deselected);

private:
    void DisableButtons();

    ArtifactsDialog* CreateDialog;

    QPushButton* AddButton;
    QPushButton* RemoveButton;
    QPushButton* MoveUpButton;
    QPushButton* MoveDownButton;
    std::array<QPushButton*, 4> Buttons;

    QListView* View;
    StructureArtifactsModel* Model;
    QItemSelectionModel* SelectionModel;
};
