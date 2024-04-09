#include "ImageArtifactsWidget.h"

#include "ArtifactsDialog.h"
#include "ImageArtifactsModel.h"
#include "ImageArtifactsView.h"
#include "PipelinesWidget.h"
#include "../ImageArtifact.h"

#include <QLabel>

ImageArtifactsWidget::ImageArtifactsWidget(QWidget* parent) :
        QWidget(parent),
        Views(new QStackedLayout()),
        CreateDialog(nullptr),
        AddChildButton(new QPushButton("Add Child Artifact")),
        AddSiblingButton(new QPushButton("Add Sibling Artifact")),
        RemoveButton(new QPushButton("Remove Artifact")),
        MoveUpButton(new QPushButton("")),
        MoveDownButton(new QPushButton("")),
        Buttons({ AddChildButton,
                  AddSiblingButton,
                  RemoveButton,
                  MoveUpButton,
                  MoveDownButton }) {

    auto* vLayout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel("Image Artifacts");
    titleLabel->setStyleSheet(PipelinesWidget::GetHeaderStyleSheet());
    vLayout->addWidget(titleLabel);

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->setContentsMargins(0, 5, 0, 0);
    MoveUpButton->setIcon(PipelinesWidget::GenerateIcon("ArrowUp"));
    MoveDownButton->setIcon(PipelinesWidget::GenerateIcon("ArrowDown"));
    buttonBarHLayout->addWidget(AddChildButton);
    buttonBarHLayout->addWidget(AddSiblingButton);
    buttonBarHLayout->addWidget(RemoveButton);
    buttonBarHLayout->addWidget(MoveDownButton);
    buttonBarHLayout->addWidget(MoveUpButton);
    connect(AddChildButton, &QPushButton::clicked, this, [this]{ AddChildArtifact(); });
    connect(AddSiblingButton, &QPushButton::clicked, this, [this]{ AddSiblingArtifact(); });
    connect(RemoveButton, &QPushButton::clicked, this, [this]{ RemoveArtifact(); });
    connect(MoveUpButton, &QPushButton::clicked, this, [this]{ MoveUp(); });
    connect(MoveDownButton, &QPushButton::clicked, this, [this]{ MoveDown(); });
    vLayout->addWidget(buttonBar);

    vLayout->addLayout(Views);
    auto* placeHolderView = new ImageArtifactsView();
    Views->addWidget(placeHolderView);
}

void ImageArtifactsWidget::SetCurrentView(int pipelineIdx) {
    if (pipelineIdx + 1 >= Views->count()) {
        qWarning("Cannot set view");
        return;
    }

    Views->setCurrentIndex(pipelineIdx + 1);
    UpdateButtonStatesOnSelectionChange(QModelIndex());
}

void ImageArtifactsWidget::AddView(Pipeline* pipeline) {
    auto* newView = new ImageArtifactsView(pipeline);
    auto* newModel = newView->model();
    newView->setHeaderHidden(true);
    Views->addWidget(newView);

    auto updateButtonStatesOnModelReset = [&, newModel, newView]() {
        DisableImageArtifactButtons();

        bool viewIsEmpty = !newModel->hasChildren(newView->rootIndex());
        if (viewIsEmpty) {
            AddChildButton->setEnabled(true);
        }
    };
    connect(newModel, &QAbstractItemModel::modelReset, this, updateButtonStatesOnModelReset);

    auto* newImageArtifactSelectionModel = newView->selectionModel();
    connect(newImageArtifactSelectionModel, &QItemSelectionModel::currentChanged,
            this, &ImageArtifactsWidget::UpdateButtonStatesOnSelectionChange);

}

void ImageArtifactsWidget::RemoveCurrentView() {
    if (Views->count() <= 1) {
        qWarning("Cannot remove any more views");
        return;
    }

    Views->removeWidget(Views->currentWidget());
}

void ImageArtifactsWidget::AddChildArtifact() {
    CreateDialog = new ImageArtifactDialog(ArtifactsDialog::CREATE, this);
    CreateDialog->show();

    connect(CreateDialog, &ArtifactsDialog::accepted, [&]() {
        auto data = ImageArtifactUi::GetWidgetData(CreateDialog);
        QModelIndex parentIndex = GetCurrentSelectionModel()->currentIndex();
        QModelIndex newIndex = GetCurrentModel()->AddChildImageArtifact(*data, parentIndex);

        GetCurrentSelectionModel()->clear();
        GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    });
}

void ImageArtifactsWidget::AddSiblingArtifact() {
    CreateDialog = new ImageArtifactDialog(ArtifactsDialog::CREATE, this);
    CreateDialog->show();

    connect(CreateDialog, &ArtifactsDialog::accepted, [&]() {
        auto data = ImageArtifactUi::GetWidgetData(CreateDialog);
        QModelIndex siblingIndex = GetCurrentSelectionModel()->currentIndex();
        QModelIndex newIndex = GetCurrentModel()->AddSiblingImageArtifact(*data, siblingIndex);

        GetCurrentSelectionModel()->clear();
        GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    });
}

void ImageArtifactsWidget::RemoveArtifact() {
    QModelIndex index = GetCurrentSelectionModel()->currentIndex();
    QModelIndex parentIndex = index.parent();
    GetCurrentModel()->RemoveImageArtifact(index);
    GetCurrentSelectionModel()->clear();
    GetCurrentSelectionModel()->setCurrentIndex(parentIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    if (!parentIndex.isValid())
        UpdateButtonStatesOnSelectionChange(parentIndex);
}

void ImageArtifactsWidget::MoveUp() {
    QModelIndex index = GetCurrentSelectionModel()->currentIndex();
    QModelIndex newIndex = GetCurrentModel()->MoveUp(index);
    GetCurrentSelectionModel()->clear();
    GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

void ImageArtifactsWidget::MoveDown() {
    QModelIndex index = GetCurrentSelectionModel()->currentIndex();
    QModelIndex newIndex = GetCurrentModel()->MoveDown(index);
    GetCurrentSelectionModel()->clear();
    GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

void ImageArtifactsWidget::UpdateButtonStatesOnSelectionChange(const QModelIndex& currentIndex) {
    bool indexIsValid = currentIndex.isValid();
    bool viewIsEmpty = !GetCurrentModel()->hasChildren(GetCurrentView()->rootIndex());

    auto* currentImageArtifact = static_cast<ImageArtifact*>(currentIndex.internalPointer());
    bool isEmptyAndInvalid = !indexIsValid && viewIsEmpty;
    bool isImageArtifactComposition = indexIsValid && currentImageArtifact->GetArtifactSubType() == Artifact::SubType::IMAGE_COMPOSITION;
    bool hasSiblingsBefore = indexIsValid && currentIndex.siblingAtRow(currentIndex.row() - 1).isValid();
    bool hasSiblingsAfter = indexIsValid && currentIndex.siblingAtRow(currentIndex.row() + 1).isValid();

    AddChildButton->setEnabled(isImageArtifactComposition || isEmptyAndInvalid);
    AddSiblingButton->setEnabled(indexIsValid);
    RemoveButton->setEnabled(indexIsValid);
    MoveUpButton->setEnabled(hasSiblingsBefore);
    MoveDownButton->setEnabled(hasSiblingsAfter);
}

void ImageArtifactsWidget::DisableImageArtifactButtons() {
    for (const auto& button: Buttons)
        button->setEnabled(false);
}

ImageArtifactsView* ImageArtifactsWidget::GetCurrentView() {
    return dynamic_cast<ImageArtifactsView*>(Views->currentWidget());
}

ImageArtifactsModel* ImageArtifactsWidget::GetCurrentModel() {
    return dynamic_cast<ImageArtifactsModel*>(GetCurrentView()->model());
}

QItemSelectionModel* ImageArtifactsWidget::GetCurrentSelectionModel() {
    return GetCurrentView()->selectionModel();
}

