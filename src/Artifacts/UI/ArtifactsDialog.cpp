#include "ArtifactsDialog.h"

#include "../Image/ImageArtifact.h"
#include "../Structure/StructureArtifact.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

ArtifactsDialog::ArtifactsDialog(QWidget* widget, Mode mode, QWidget* parent) :
        QDialog(parent),
        VLayout(new QVBoxLayout(this)) {

    if (!widget)
        throw std::runtime_error("Given widget cannot be nullptr");

    VLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setMinimumSize(600, 400);
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    setModal(true);

    setWindowTitle(mode == Mode::EDIT ? "Edit Image Artifact" : "Create Image Artifact");

    VLayout->setAlignment(Qt::AlignTop);

    VLayout->addWidget(widget);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    VLayout->addStretch();
    VLayout->addSpacing(20);
    VLayout->addWidget(dialogButtonBar);

    if (mode == Mode::CREATE) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }
}

ImageArtifactDialog::ImageArtifactDialog(ArtifactsDialog::Mode mode, QWidget* parent) :
        ArtifactsDialog(new ImageArtifactWidget(), mode, parent) {}

StructureArtifactDialog::StructureArtifactDialog(ArtifactsDialog::Mode mode, QWidget* parent) :
        ArtifactsDialog(new StructureArtifactWidget(), mode, parent) {}
