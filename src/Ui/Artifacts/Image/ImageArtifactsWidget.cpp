#include "ImageArtifactsWidget.h"

#include "ImageArtifactsModel.h"
#include "ImageArtifactsView.h"
#include "../ArtifactsDialog.h"
#include "../PipelinesWidget.h"
#include "../../Utils/WidgetUtils.h"
#include "../../../Artifacts/Image/ImageArtifact.h"

#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QBoxLayout>

ImageArtifactsWidget::ImageArtifactsWidget(QWidget* parent) :
        QWidget(parent),
        Views(new QStackedLayout()),
        CreateDialog(nullptr),
        AddChildButton(new QPushButton(GenerateIcon("Plus"), " Child")),
        AddSiblingButton(new QPushButton(GenerateIcon("Plus"), " Sibling")),
        RemoveButton(new QPushButton(GenerateIcon("Minus"), "")),
        MoveUpButton(new QPushButton(GenerateIcon("ArrowUp"), "")),
        MoveDownButton(new QPushButton(GenerateIcon("ArrowDown"), "")),
        Buttons({ AddChildButton,
                  AddSiblingButton,
                  RemoveButton,
                  MoveUpButton,
                  MoveDownButton }) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});

    auto* titleLabel = new QLabel("Image Artifacts");
    titleLabel->setStyleSheet(GetHeader2StyleSheet());
    vLayout->addWidget(titleLabel);

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->setContentsMargins(0, 5, 0, 0);
    buttonBarHLayout->addWidget(MoveDownButton);
    buttonBarHLayout->addWidget(MoveUpButton);
    buttonBarHLayout->addSpacing(5);
    buttonBarHLayout->addStretch();
    buttonBarHLayout->addSpacing(5);
    buttonBarHLayout->addWidget(AddChildButton);
    buttonBarHLayout->addWidget(AddSiblingButton);
    buttonBarHLayout->addWidget(RemoveButton);
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

void ImageArtifactsWidget::AddView(Pipeline const& pipeline) {
    auto* newView = new ImageArtifactsView(&pipeline);
    auto* newModel = newView->model();
    newView->setHeaderHidden(true);
    Views->addWidget(newView);

    auto updateButtonStatesOnModelReset = [&, newModel, newView] {
        DisableImageArtifactButtons();

        if (!newModel->hasChildren(newView->rootIndex())) {
            AddChildButton->setEnabled(true);
        }
    };
    connect(newModel, &QAbstractItemModel::modelReset, this, updateButtonStatesOnModelReset);

    auto const* newImageArtifactSelectionModel = newView->selectionModel();
    connect(newImageArtifactSelectionModel, &QItemSelectionModel::currentChanged,
            this, &ImageArtifactsWidget::UpdateButtonStatesOnSelectionChange);

}

void ImageArtifactsWidget::RemoveCurrentView() const {
    if (Views->count() <= 1) {
        qWarning("Cannot remove any more views");
        return;
    }

    Views->removeWidget(Views->currentWidget());
}

void ImageArtifactsWidget::AddChildArtifact() {
    CreateDialog = new ImageArtifactDialog(ArtifactsDialog::Mode::CREATE, this);
    CreateDialog->show();

    connect(CreateDialog, &ArtifactsDialog::accepted, [&] {
        auto const data = ImageArtifactWidget::GetWidgetData(CreateDialog);
        QModelIndex const parentIndex = GetCurrentSelectionModel()->currentIndex();
        QModelIndex const newIndex = GetCurrentModel()->AddChildImageArtifact(data, parentIndex);

        GetCurrentSelectionModel()->clear();
        GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    });
}

void ImageArtifactsWidget::AddSiblingArtifact() {
    CreateDialog = new ImageArtifactDialog(ArtifactsDialog::Mode::CREATE, this);

    CreateDialog->updateGeometry();
    CreateDialog->show();

    connect(CreateDialog, &ArtifactsDialog::accepted, [&] {
        auto const data = ImageArtifactWidget::GetWidgetData(CreateDialog);
        QModelIndex const siblingIndex = GetCurrentSelectionModel()->currentIndex();
        QModelIndex const newIndex = GetCurrentModel()->AddSiblingImageArtifact(data, siblingIndex);

        GetCurrentSelectionModel()->clear();
        GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    });
}

void ImageArtifactsWidget::RemoveArtifact() {
    QModelIndex const index = GetCurrentSelectionModel()->currentIndex();
    QModelIndex const parentIndex = index.parent();
    GetCurrentModel()->RemoveImageArtifact(index);
    GetCurrentSelectionModel()->clear();
    GetCurrentSelectionModel()->setCurrentIndex(parentIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
    if (!parentIndex.isValid())
        UpdateButtonStatesOnSelectionChange(parentIndex);
}

void ImageArtifactsWidget::MoveUp() {
    QModelIndex const index = GetCurrentSelectionModel()->currentIndex();
    QModelIndex const newIndex = GetCurrentModel()->MoveUp(index);
    GetCurrentSelectionModel()->clear();
    GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

void ImageArtifactsWidget::MoveDown() {
    QModelIndex const index = GetCurrentSelectionModel()->currentIndex();
    QModelIndex const newIndex = GetCurrentModel()->MoveDown(index);
    GetCurrentSelectionModel()->clear();
    GetCurrentSelectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

void ImageArtifactsWidget::UpdateButtonStatesOnSelectionChange(const QModelIndex& currentIndex) {
    bool const indexIsValid = currentIndex.isValid();
    bool const viewIsEmpty = !GetCurrentModel()->hasChildren(GetCurrentView()->rootIndex());

    bool const isEmptyAndInvalid = !indexIsValid && viewIsEmpty;
    bool const isImageArtifactComposition = indexIsValid
            && std::holds_alternative<CompositeImageArtifactData>(
                    currentIndex.data(Qt::UserRole).value<ImageArtifactData>().Data);
    bool const hasSiblingsBefore = indexIsValid && currentIndex.siblingAtRow(currentIndex.row() - 1).isValid();
    bool const hasSiblingsAfter = indexIsValid && currentIndex.siblingAtRow(currentIndex.row() + 1).isValid();

    AddChildButton->setEnabled(isImageArtifactComposition || isEmptyAndInvalid);
    AddSiblingButton->setEnabled(indexIsValid);
    RemoveButton->setEnabled(indexIsValid);
    MoveUpButton->setEnabled(hasSiblingsBefore);
    MoveDownButton->setEnabled(hasSiblingsAfter);
}

void ImageArtifactsWidget::DisableImageArtifactButtons() const {
    for (const auto& button: Buttons)
        button->setEnabled(false);
}

ImageArtifactsView* ImageArtifactsWidget::GetCurrentView() const {
    return dynamic_cast<ImageArtifactsView*>(Views->currentWidget());
}

ImageArtifactsModel* ImageArtifactsWidget::GetCurrentModel() const {
    return dynamic_cast<ImageArtifactsModel*>(GetCurrentView()->model());
}

QItemSelectionModel* ImageArtifactsWidget::GetCurrentSelectionModel() const {
    return GetCurrentView()->selectionModel();
}

