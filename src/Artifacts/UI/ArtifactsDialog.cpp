#include "ArtifactsDialog.h"

#include "../ImageArtifact.h"

#include <QDialogButtonBox>

ArtifactsDialog::ArtifactsDialog(Mode mode, QWidget* parent) :
        QDialog(parent) {

    setMinimumSize(200, 100);

    setModal(true);

    setWindowTitle(mode == EDIT ? "Edit Image Artifact" : "Create Image Artifact");

    auto vLayout = new QVBoxLayout(this);
    vLayout->setAlignment(Qt::AlignTop);

    auto widget = ImageArtifactUi::GetWidget(mode == CREATE);
    vLayout->addWidget(widget);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    vLayout->addStretch();
    vLayout->addSpacing(20);
    vLayout->addWidget(dialogButtonBar);

    if (mode == CREATE) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }
}
