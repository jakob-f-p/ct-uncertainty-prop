#include "ArtifactsDialog.h"

#include "../ImageArtifact.h"
#include "../StructureArtifact.h"

#include <QDialogButtonBox>

ArtifactsDialog::ArtifactsDialog(Mode mode, QWidget* parent) :
        QDialog(parent),
        VLayout(new QVBoxLayout(this)) {

    setMinimumSize(200, 100);

    setModal(true);

    setWindowTitle(mode == EDIT ? "Edit Image Artifact" : "Create Image Artifact");

    VLayout->setAlignment(Qt::AlignTop);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    VLayout->addStretch();
    VLayout->addSpacing(20);
    VLayout->addWidget(dialogButtonBar);

    if (mode == CREATE) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }
}



template class ArtifactsTypeDialog<ImageArtifactUi>;
template class ArtifactsTypeDialog<StructureArtifactUi>;

template<typename Ui>
ArtifactsTypeDialog<Ui>::ArtifactsTypeDialog(ArtifactsDialog::Mode mode, QWidget* parent) :
        ArtifactsDialog(mode, parent) {
    auto* widget = Ui::GetWidget(mode == CREATE);
    VLayout->insertWidget(0, widget);
}
