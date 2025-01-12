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
                                            std::string const& title,
                                            QWidget* parent = nullptr);

protected Q_SLOTS:
    void AddArtifact();
    void RemoveArtifact() const;
    void MoveUp() const;
    void MoveDown() const;
    void UpdateButtonStatesOnSelectionChange(const QItemSelection& selected, const QItemSelection& deselected) const;

private:
    void DisableButtons() const;

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
