#include "ArtifactsDialog.h"

#include "../ImageArtifact.h"
#include "../StructureArtifact.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

ArtifactsDialog::ArtifactsDialog(Mode mode, QWidget* parent) :
        QDialog(parent),
        VLayout(new QVBoxLayout(this)) {

    VLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setMinimumSize(450, 200);
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    setModal(true);

    setWindowTitle(mode == Mode::EDIT ? "Edit Image Artifact" : "Create Image Artifact");

    VLayout->setAlignment(Qt::AlignTop);

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



template class ArtifactsTypeDialog<ImageArtifactWidget>;
template class ArtifactsTypeDialog<StructureArtifactWidget>;

template<typename Widget>
ArtifactsTypeDialog<Widget>::ArtifactsTypeDialog(ArtifactsDialog::Mode mode, QWidget* parent) :
        ArtifactsDialog(mode, parent) {
    VLayout->insertWidget(0, new Widget());
}
