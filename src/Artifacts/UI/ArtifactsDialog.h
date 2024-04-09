#pragma once

#include <QDialog>
#include <QVBoxLayout>

class ArtifactsDialog : public QDialog {
public:
    enum Mode {
        EDIT,
        CREATE
    };

protected:
    explicit ArtifactsDialog(Mode mode, QWidget* parent = nullptr);

    QVBoxLayout* VLayout;
};


template<typename Ui>
class ArtifactsTypeDialog : public ArtifactsDialog {
public:
    explicit ArtifactsTypeDialog(Mode mode, QWidget* parent = nullptr);
};

class ImageArtifactUi;
class StructureArtifactUi;

typedef ArtifactsTypeDialog<ImageArtifactUi> ImageArtifactDialog;
typedef ArtifactsTypeDialog<StructureArtifactUi> StructureArtifactDialog;
