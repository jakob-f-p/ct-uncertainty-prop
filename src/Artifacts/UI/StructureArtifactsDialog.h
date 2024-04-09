#pragma once

#include <QDialog>
#include <QListView>
#include <QStackedLayout>

class ArtifactsDialog;

class StructureArtifactsDialog : public QDialog {
public:
    explicit StructureArtifactsDialog(QWidget* parent = nullptr);

protected slots:
    void AddArtifact();
    void RemoveArtifact();
    void MoveUp();
    void MoveDown();
    void UpdateButtonStatesOnSelectionChange(const QModelIndex& currentIndex);

private:
    void DisableImageArtifactButtons();

    ArtifactsDialog* CreateDialog;

    QPushButton* AddButton;
    QPushButton* RemoveButton;
    QPushButton* MoveUpButton;
    QPushButton* MoveDownButton;
    std::array<QPushButton*, 4> Buttons;

    QListView* View;
    QAbstractItemModel* Model;
    QItemSelectionModel* SelectionModel;
};
