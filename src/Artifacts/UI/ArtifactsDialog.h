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
    explicit ArtifactsDialog(Mode mode, QWidget* parent = nullptr);

    QVBoxLayout* VLayout;
};


template<typename Widget>
class ArtifactsTypeDialog : public ArtifactsDialog {
public:
    explicit ArtifactsTypeDialog(Mode mode, QWidget* parent = nullptr);
};

class ImageArtifactWidget;
class StructureArtifactWidget;

using ImageArtifactDialog = ArtifactsTypeDialog<ImageArtifactWidget>;
using StructureArtifactDialog = ArtifactsTypeDialog<StructureArtifactWidget>;
