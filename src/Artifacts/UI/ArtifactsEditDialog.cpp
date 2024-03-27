#include "ArtifactsEditDialog.h"

#include "../ImageArtifact.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>

ArtifactsEditDialog::ArtifactsEditDialog(Mode mode, QWidget* parent, ImageArtifact* imageArtifact) :
        QDialog(parent),
        ImageArt(imageArtifact),
        ImageArtifactEditWidget(nullptr),
        ImageArtifactTypeComboBox(nullptr),
        VLayout(new QVBoxLayout(this)),
        DialogMode(mode) {

    setMinimumSize(200, 100);

    setModal(true);

    setWindowTitle(DialogMode == EDIT ? "Edit Image Artifact" : "Create Image Artifact");

    VLayout->setAlignment(Qt::AlignTop);

    if (DialogMode == CREATE) {
        auto* imageArtifactTypeSelectionBar = new QWidget();
        auto* hLayout = new QHBoxLayout(imageArtifactTypeSelectionBar);
        auto* label = new QLabel("Image Artifact Type");
        hLayout->addWidget(label);
        ImageArtifactTypeComboBox = new QComboBox();
        hLayout->addWidget(ImageArtifactTypeComboBox);
        for (const auto& imageArtifactType: Artifact::GetImageArtifactTypes()) {
            std::string fullName = Artifact::SubTypeToString(imageArtifactType);
            std::string comboBoxName = fullName.erase(0, fullName.find(' ') + 1);
            ImageArtifactTypeComboBox->addItem(QString::fromStdString(comboBoxName),
                                               imageArtifactType);
        }

        VLayout->addWidget(imageArtifactTypeSelectionBar);

        ImageArt = dynamic_cast<ImageArtifact*>(Artifact::NewArtifact(Artifact::IMAGE_GAUSSIAN)); // default artifact
        if (int idx = ImageArtifactTypeComboBox->findData(ImageArt->GetArtifactSubType());
                idx != -1) {
            ImageArtifactTypeComboBox->setCurrentIndex(idx);
        }

        connect(ImageArtifactTypeComboBox, &QComboBox::currentIndexChanged,
                this, &ArtifactsEditDialog::UpdateFormAccordingToImageArtifactType);
    }

    ImageArtifactEditWidget = ImageArt->GetEditWidget();
    VLayout->addWidget(ImageArtifactEditWidget);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    VLayout->addStretch();
    VLayout->addSpacing(20);
    VLayout->addWidget(dialogButtonBar);

    if (DialogMode == CREATE) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }
}

void ArtifactsEditDialog::UpdateFormAccordingToImageArtifactType() {
    auto newSubType = ImageArtifactTypeComboBox->currentData().value<Artifact::SubType>();
    auto* newBlankImageArtifact = dynamic_cast<ImageArtifact*>(Artifact::NewArtifact(newSubType));

    layout()->removeWidget(ImageArtifactEditWidget);
    delete ImageArtifactEditWidget;
    ImageArt->Delete();

    ImageArt = newBlankImageArtifact;
    ImageArtifactEditWidget = ImageArt->GetEditWidget();
    VLayout->insertWidget(1, ImageArtifactEditWidget);
}

ArtifactsEditDialog::~ArtifactsEditDialog() {
    if (ImageArt && DialogMode == CREATE)
        ImageArt->Delete();
}

ImageArtifact* ArtifactsEditDialog::GetNewImageArtifact() {
    return ImageArt;
}
