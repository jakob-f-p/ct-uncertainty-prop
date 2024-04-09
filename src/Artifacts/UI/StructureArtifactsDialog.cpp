#include "StructureArtifactsDialog.h"

#include "ArtifactsDialog.h"
#include "ImageArtifactsModel.h"
#include "ImageArtifactsView.h"
#include "PipelinesWidget.h"
#include "../ImageArtifact.h"

#include <QLabel>

StructureArtifactsDialog::StructureArtifactsDialog(QWidget* parent) :
        QDialog(parent),
        CreateDialog(nullptr),
        View(),
        Model(),
        SelectionModel(),
        AddButton(new QPushButton("")),
        RemoveButton(new QPushButton("")),
        MoveUpButton(new QPushButton("")),
        MoveDownButton(new QPushButton("")),
        Buttons({ AddButton,
                  RemoveButton,
                  MoveUpButton,
                  MoveDownButton }) {

    auto* vLayout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel("Structure Artifacts");
    titleLabel->setStyleSheet(PipelinesWidget::GetHeaderStyleSheet());
    vLayout->addWidget(titleLabel);

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->setContentsMargins(0, 5, 0, 0);
    MoveUpButton->setIcon(PipelinesWidget::GenerateIcon("ArrowUp"));
    MoveDownButton->setIcon(PipelinesWidget::GenerateIcon("ArrowDown"));
    buttonBarHLayout->addWidget(AddButton);
    buttonBarHLayout->addWidget(RemoveButton);
    buttonBarHLayout->addWidget(MoveDownButton);
    buttonBarHLayout->addWidget(MoveUpButton);
    connect(AddButton, &QPushButton::clicked, this, [this]{ AddArtifact(); });
    connect(RemoveButton, &QPushButton::clicked, this, [this]{ RemoveArtifact(); });
    connect(MoveUpButton, &QPushButton::clicked, this, [this]{ MoveUp(); });
    connect(MoveDownButton, &QPushButton::clicked, this, [this]{ MoveDown(); });
    vLayout->addWidget(buttonBar);
}

void StructureArtifactsDialog::AddArtifact() {
    CreateDialog = new ImageArtifactDialog(ArtifactsDialog::CREATE, this);
    CreateDialog->show();

    connect(CreateDialog, &ArtifactsDialog::accepted, [&]() {
        auto data = ImageArtifactUi::GetWidgetData(CreateDialog);
        QModelIndex parentIndex = SelectionModel->currentIndex();
//        QModelIndex newIndex = Model->AddChildImageArtifact(*data, parentIndex);

        SelectionModel->clear();
//        SelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    });
}

void StructureArtifactsDialog::RemoveArtifact() {
    QModelIndex index = SelectionModel->currentIndex();
    QModelIndex parentIndex = index.parent();
//    Model->RemoveImageArtifact(index);
    SelectionModel->clear();
    SelectionModel->setCurrentIndex(parentIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    if (!parentIndex.isValid())
        UpdateButtonStatesOnSelectionChange(parentIndex);
}

void StructureArtifactsDialog::MoveUp() {
    QModelIndex index = SelectionModel->currentIndex();
//    QModelIndex newIndex = Model->MoveUp(index);
    SelectionModel->clear();
//    SelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

void StructureArtifactsDialog::MoveDown() {
    QModelIndex index = SelectionModel->currentIndex();
//    QModelIndex newIndex = Model->MoveDown(index);
    SelectionModel->clear();
//    SelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

void StructureArtifactsDialog::UpdateButtonStatesOnSelectionChange(const QModelIndex& currentIndex) {
    bool indexIsValid = currentIndex.isValid();
    bool viewIsEmpty = Model->hasChildren(currentIndex);

    auto* currentImageArtifact = static_cast<ImageArtifact*>(currentIndex.internalPointer());
    bool isEmptyAndInvalid = !indexIsValid && viewIsEmpty;
    bool isImageArtifactComposition = indexIsValid && currentImageArtifact->GetArtifactSubType() == Artifact::SubType::IMAGE_COMPOSITION;
    bool hasSiblingsBefore = indexIsValid && currentIndex.siblingAtRow(currentIndex.row() - 1).isValid();
    bool hasSiblingsAfter = indexIsValid && currentIndex.siblingAtRow(currentIndex.row() + 1).isValid();

    AddButton->setEnabled(isImageArtifactComposition || isEmptyAndInvalid);
    RemoveButton->setEnabled(indexIsValid);
    MoveUpButton->setEnabled(hasSiblingsBefore);
    MoveDownButton->setEnabled(hasSiblingsAfter);
}

void StructureArtifactsDialog::DisableImageArtifactButtons() {

}
