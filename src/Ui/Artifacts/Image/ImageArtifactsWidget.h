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
    void AddView(Pipeline const& pipeline);
    void RemoveCurrentView() const;

protected Q_SLOTS:
    void AddChildArtifact();
    void AddSiblingArtifact();
    void RemoveArtifact();
    void MoveUp();
    void MoveDown();
    void UpdateButtonStatesOnSelectionChange(const QModelIndex& currentIndex);

private:
    void DisableImageArtifactButtons() const;

    auto GetCurrentView() const -> ImageArtifactsView*;
    auto GetCurrentModel() const -> ImageArtifactsModel*;
    auto GetCurrentSelectionModel() const -> QItemSelectionModel*;

    QStackedLayout* Views;

    ArtifactsDialog* CreateDialog;

    QPushButton* AddChildButton;
    QPushButton* AddSiblingButton;
    QPushButton* RemoveButton;
    QPushButton* MoveUpButton;
    QPushButton* MoveDownButton;
    std::array<QPushButton*, 5> Buttons;
};
