#pragma once

#include <QDialog>

class ArtifactsDialog : public QDialog {

public:
    enum Mode {
        EDIT,
        CREATE
    };

    explicit ArtifactsDialog(Mode mode, QWidget* parent = nullptr);
};
