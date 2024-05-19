#pragma once

#include <QDialog>

class QVBoxLayout;

class ArtifactsDialog : public QDialog {
public:
    enum struct Mode : uint8_t {
        EDIT,
        CREATE
    };

protected:
    explicit ArtifactsDialog(QWidget* widget, Mode mode, QWidget* parent = nullptr);

private:
    QVBoxLayout* VLayout;
};

class ImageArtifactDialog : public ArtifactsDialog {
public:
    explicit ImageArtifactDialog(Mode mode, QWidget* parent = nullptr);
};

class StructureArtifactDialog : public ArtifactsDialog {
public:
    explicit StructureArtifactDialog(Mode mode, QWidget* parent = nullptr);
};
