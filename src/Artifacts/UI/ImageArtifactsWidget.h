#pragma once

#include <QWidget>

class ArtifactsDialog;
class ImageArtifactsModel;
class ImageArtifactsView;
class Pipeline;

class QItemSelectionModel;
class QPushButton;
class QStackedLayout;

class ImageArtifactsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImageArtifactsWidget(QWidget* parent = nullptr);

    void SetCurrentView(int pipelineIdx);
    void AddView(Pipeline& pipeline);
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

    auto GetCurrentView() -> ImageArtifactsView*;
    auto GetCurrentModel() -> ImageArtifactsModel*;
    auto GetCurrentSelectionModel() -> QItemSelectionModel*;

    QStackedLayout* Views;

    ArtifactsDialog* CreateDialog;

    QPushButton* AddChildButton;
    QPushButton* AddSiblingButton;
    QPushButton* RemoveButton;
    QPushButton* MoveUpButton;
    QPushButton* MoveDownButton;
    std::array<QPushButton*, 5> Buttons;
};
