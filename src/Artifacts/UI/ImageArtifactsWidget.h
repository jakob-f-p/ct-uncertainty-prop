#pragma once

#include <QItemSelectionModel>
#include <QPushButton>
#include <QWidget>
#include <QStackedLayout>

class ArtifactsDialog;
class ImageArtifactsModel;
class ImageArtifactsView;
class Pipeline;

class ImageArtifactsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImageArtifactsWidget(QWidget* parent = nullptr);

    void SetCurrentView(int pipelineIdx);
    void AddView(Pipeline* pipeline);
    void RemoveCurrentView();

protected slots:
    void AddChildArtifact();
    void AddSiblingArtifact();
    void RemoveArtifact();
    void MoveUp();
    void MoveDown();
    void UpdateButtonStatesOnSelectionChange(const QModelIndex& currentIndex);

private:
    void DisableImageArtifactButtons();

    ImageArtifactsView* GetCurrentView();
    ImageArtifactsModel* GetCurrentModel();
    QItemSelectionModel* GetCurrentSelectionModel();

    QStackedLayout* Views;

    ArtifactsDialog* CreateDialog;

    QPushButton* AddChildButton;
    QPushButton* AddSiblingButton;
    QPushButton* RemoveButton;
    QPushButton* MoveUpButton;
    QPushButton* MoveDownButton;
    std::array<QPushButton*, 5> Buttons;
};
