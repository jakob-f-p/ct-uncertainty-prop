#include "StructureArtifactsWidgetDialog.h"

#include "../ArtifactsDialog.h"
#include "../PipelinesWidget.h"
#include "StructureArtifactsView.h"
#include "StructureArtifactsModel.h"
#include "../../../Artifacts/Structure/StructureArtifact.h"

#include <QItemSelectionModel>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>

StructureArtifactsWidgetDialog::StructureArtifactsWidgetDialog(StructureArtifactList& structureWrapper,
                                                               std::string const& title,
                                                               QWidget* parent) :
        QDialog(parent),
        CreateDialog(nullptr),
        AddButton(new QPushButton("")),
        RemoveButton(new QPushButton("")),
        MoveUpButton(new QPushButton("")),
        MoveDownButton(new QPushButton("")),
        Buttons({ AddButton,
            RemoveButton,
            MoveUpButton,
            MoveDownButton }),
        View(new StructureArtifactsView(structureWrapper)),
        Model(dynamic_cast<StructureArtifactsModel*>(View->model())),
        SelectionModel(View->selectionModel()) {

    setMinimumSize(300, 300);

    setWindowTitle("Structure Artifacts");

    auto* vLayout = new QVBoxLayout(this);

    auto* titleBarHLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel(QString::fromStdString(title));
    titleLabel->setStyleSheet(GetHeader1StyleSheet());
    titleBarHLayout->addWidget(titleLabel);

    titleBarHLayout->addStretch();

    auto* buttonBar = new QWidget();
    auto* buttonBarHLayout = new QHBoxLayout(buttonBar);
    buttonBarHLayout->setContentsMargins(0, 5, 0, 0);
    AddButton->setIcon(GenerateIcon("Plus"));
    RemoveButton->setIcon(GenerateIcon("Minus"));
    MoveUpButton->setIcon(GenerateIcon("ArrowUp"));
    MoveDownButton->setIcon(GenerateIcon("ArrowDown"));
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

    vLayout->addWidget(View);

    connect(SelectionModel, &QItemSelectionModel::selectionChanged,
            this, &StructureArtifactsWidgetDialog::UpdateButtonStatesOnSelectionChange);
}

void StructureArtifactsWidgetDialog::AddArtifact() {
    CreateDialog = new StructureArtifactDialog(ArtifactsDialog::Mode::CREATE, this);
    CreateDialog->show();

    connect(CreateDialog, &ArtifactsDialog::accepted, [&] {
        auto const data = GetWidgetData<StructureArtifactWidget>(CreateDialog);
        QModelIndex const parentIndex = SelectionModel->currentIndex();
        QModelIndex const newIndex = Model->AddStructureArtifact(data, parentIndex);

        SelectionModel->clearSelection();
        SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
        if (Model->rowCount({}) == 1)
            RemoveButton->setDisabled(true);
    });
}

void StructureArtifactsWidgetDialog::RemoveArtifact() const {
    QModelIndex const index = SelectionModel->selection().indexes().at(0);
    assert(SelectionModel->selection().indexes().size() == 1);

    Model->RemoveStructureArtifact(index);

    SelectionModel->clearSelection();
}

void StructureArtifactsWidgetDialog::MoveUp() const {
    QModelIndex const index = SelectionModel->selection().indexes().at(0);
    assert(SelectionModel->selection().indexes().size() == 1);

    QModelIndex const newIndex = Model->MoveUp(index);

    SelectionModel->clearSelection();
    SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
}

void StructureArtifactsWidgetDialog::MoveDown() const {
    QModelIndex const index = SelectionModel->selection().indexes().at(0);
    assert(SelectionModel->selection().indexes().size() == 1);

    QModelIndex const newIndex = Model->MoveDown(index);

    SelectionModel->clearSelection();
    SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
}

void StructureArtifactsWidgetDialog::UpdateButtonStatesOnSelectionChange(const QItemSelection& selected,
                                                                         const QItemSelection&) const {
    auto const modelIndexList = selected.indexes();
    if (modelIndexList.size() > 1)
        throw std::runtime_error("Invalid selection");

    QModelIndex const selectedIndex = modelIndexList.size() == 1
            ? selected.indexes()[0]
            : QModelIndex();
    bool const indexIsValid = selectedIndex.isValid();
    bool const hasSiblingsBefore = indexIsValid && selectedIndex.siblingAtRow(selectedIndex.row() - 1).isValid();
    bool const hasSiblingsAfter = indexIsValid && selectedIndex.siblingAtRow(selectedIndex.row() + 1).isValid();

    AddButton->setEnabled(true);
    RemoveButton->setEnabled(indexIsValid);
    MoveUpButton->setEnabled(hasSiblingsBefore);
    MoveDownButton->setEnabled(hasSiblingsAfter);
}

void StructureArtifactsWidgetDialog::DisableButtons() const {
    for (const auto& button: Buttons)
        button->setEnabled(false);
}
