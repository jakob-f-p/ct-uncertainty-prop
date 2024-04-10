#include "StructureArtifactsWidgetDialog.h"

#include "ArtifactsDialog.h"
#include "PipelinesWidget.h"
#include "StructureArtifactsDelegate.h"
#include "StructureArtifactsModel.h"
#include "../StructureArtifact.h"
#include "../StructureWrapper.h"

#include <QLabel>
#include <QPushButton>

StructureArtifactsWidgetDialog::StructureArtifactsWidgetDialog(ArtifactStructureWrapper& structureWrapper, QWidget* parent) :
        QDialog(parent),
        CreateDialog(nullptr),
        View(new QListView()),
        Model(new StructureArtifactsModel(structureWrapper)),
        SelectionModel(),
        AddButton(new QPushButton("")),
        RemoveButton(new QPushButton("")),
        MoveUpButton(new QPushButton("")),
        MoveDownButton(new QPushButton("")),
        Buttons({ AddButton,
                  RemoveButton,
                  MoveUpButton,
                  MoveDownButton }) {

    setMinimumSize(300, 300);

    setWindowTitle("Structure Artifacts");

    auto* vLayout = new QVBoxLayout(this);

    auto* titleBarHLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel(QString::fromStdString(structureWrapper.GetViewName()));
    titleLabel->setStyleSheet(PipelinesWidget::GetHeaderStyleSheet());
    titleBarHLayout->addWidget(titleLabel);

    titleBarHLayout->addStretch();

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->setContentsMargins(0, 5, 0, 0);
    AddButton->setIcon(PipelinesWidget::GenerateIcon("Plus"));
    RemoveButton->setIcon(PipelinesWidget::GenerateIcon("Minus"));
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
    DisableButtons();
    AddButton->setEnabled(true);
    titleBarHLayout->addWidget(buttonBar);
    vLayout->addLayout(titleBarHLayout);

    View->setModel(Model);
    View->setItemDelegate(new StructureArtifactsDelegate());
    vLayout->addWidget(View);
    SelectionModel = View->selectionModel();

    connect(SelectionModel, &QItemSelectionModel::selectionChanged,
            this, &StructureArtifactsWidgetDialog::UpdateButtonStatesOnSelectionChange);
}

void StructureArtifactsWidgetDialog::AddArtifact() {
    CreateDialog = new StructureArtifactDialog(ArtifactsDialog::CREATE, this);
    CreateDialog->show();

    connect(CreateDialog, &ArtifactsDialog::accepted, [&]() {
        auto data = StructureArtifactUi::GetWidgetData(CreateDialog);
        QModelIndex parentIndex = SelectionModel->currentIndex();
        QModelIndex newIndex = Model->AddStructureArtifact(*data, parentIndex);

        SelectionModel->clearSelection();
        SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
        if (Model->rowCount({}) == 1)
            RemoveButton->setDisabled(true);
    });
}

void StructureArtifactsWidgetDialog::RemoveArtifact() {
    QModelIndex index = SelectionModel->currentIndex();
    Model->RemoveStructureArtifact(index);
    SelectionModel->clearSelection();
}

void StructureArtifactsWidgetDialog::MoveUp() {
    QModelIndex index = SelectionModel->currentIndex();
    QModelIndex newIndex = Model->MoveUp(index);
    SelectionModel->clearSelection();
    SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
}

void StructureArtifactsWidgetDialog::MoveDown() {
    QModelIndex index = SelectionModel->currentIndex();
    QModelIndex newIndex = Model->MoveDown(index);
    SelectionModel->clearSelection();
    SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
}

void StructureArtifactsWidgetDialog::UpdateButtonStatesOnSelectionChange(const QItemSelection& selected,
                                                                         const QItemSelection&) {
    auto modelIndexList = selected.indexes();
    std::cout << "hi" << std::endl;
    if (modelIndexList.size() > 1) {
        qWarning("Invalid selection");
        return;
    }
    QModelIndex selectedIndex = modelIndexList.size() == 1
            ? selected.indexes()[0]
            : QModelIndex();
    bool indexIsValid = selectedIndex.isValid();
    bool hasSiblingsBefore = indexIsValid && selectedIndex.siblingAtRow(selectedIndex.row() - 1).isValid();
    bool hasSiblingsAfter = indexIsValid && selectedIndex.siblingAtRow(selectedIndex.row() + 1).isValid();

    AddButton->setEnabled(true);
    RemoveButton->setEnabled(indexIsValid);
    MoveUpButton->setEnabled(hasSiblingsBefore);
    MoveDownButton->setEnabled(hasSiblingsAfter);
}

void StructureArtifactsWidgetDialog::DisableButtons() {
    for (const auto& button: Buttons)
        button->setEnabled(false);
}
