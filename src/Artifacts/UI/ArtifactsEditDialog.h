#pragma once

#include <QComboBox>
#include <QDialog>
#include <QVBoxLayout>

class ImageArtifact;

class ArtifactsEditDialog : public QDialog {
    Q_OBJECT

public:
    enum Mode {
        EDIT,
        CREATE
    };

    explicit ArtifactsEditDialog(Mode mode, QWidget* parent = nullptr, ImageArtifact* imageArtifact = nullptr);
    ~ArtifactsEditDialog() override;

    ImageArtifact* GetNewImageArtifact();

protected slots:
    void UpdateFormAccordingToImageArtifactType();

protected:
    ImageArtifact* ImageArt;
    QWidget* ImageArtifactEditWidget;
    QComboBox* ImageArtifactTypeComboBox;
    QVBoxLayout* VLayout;
    Mode DialogMode;
};
