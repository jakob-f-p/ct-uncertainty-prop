#pragma once

#include <QDialog>
#include <QListView>
#include <QStackedLayout>

class ArtifactsDialog;
class StructureArtifacts;
class StructureArtifactsModel;

class StructureArtifactsWidgetDialog : public QDialog {
public:
    explicit StructureArtifactsWidgetDialog(StructureArtifacts& structureWrapper,
                                            std::string& title,
                                            QWidget* parent = nullptr);

protected slots:
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
