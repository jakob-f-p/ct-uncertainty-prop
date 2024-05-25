#include "StructureArtifactsView.h"

#include "StructureArtifactsModel.h"
#include "../ArtifactsDialog.h"
#include "../../Utils/ModelUtils.h"
#include "../../../Artifacts/Structure/StructureArtifact.h"

StructureArtifactsView::StructureArtifactsView(StructureArtifactList& structureArtifactList) {
    setModel(new StructureArtifactsModel(structureArtifactList));
    setItemDelegate(new StructureArtifactsDelegate());
}

StructureArtifactsView::StructureArtifactsDelegate::StructureArtifactsDelegate(QObject* parent) :
        DialogDelegate(parent) {}

auto
StructureArtifactsView::StructureArtifactsDelegate::getDialog(QModelIndex const& index,
                                                              QWidget* parent) const noexcept -> QDialog* {
    return new StructureArtifactDialog(ArtifactsDialog::Mode::EDIT, parent);
}

void StructureArtifactsView::StructureArtifactsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto data = index.data(Qt::UserRole).value<StructureArtifactData>();

    SetWidgetData<StructureArtifactWidget>(editor, data);
}

void StructureArtifactsView::StructureArtifactsDelegate::setModelData(QWidget* editor,
                                                                      QAbstractItemModel* model,
                                                                      const QModelIndex& index) const {
    auto data = GetWidgetData<StructureArtifactWidget>(editor);

    model->setData(index, QVariant::fromValue(data));
}

StructureArtifactsReadOnlyView::StructureArtifactsReadOnlyView(StructureArtifactList const& structureArtifactList) :
        Model(new StructureArtifactsReadOnlyModel(structureArtifactList)) {

    setModel(Model);
}

auto StructureArtifactsReadOnlyView::Select(StructureArtifact const& structureArtifact) -> void {
    auto structureArtifactPointer = QVariant::fromValue(const_cast<StructureArtifact*>(&structureArtifact));
    auto match = Search(*Model, StructureArtifactsModel::Roles::POINTER,
                        QVariant::fromValue(structureArtifactPointer), rootIndex());

    if (match == QModelIndex{})
        throw std::runtime_error("Structure artifact not found");

    selectionModel()->clearSelection();
    selectionModel()->select(match, QItemSelectionModel::SelectionFlag::Select);
}

void StructureArtifactsReadOnlyView::selectionChanged(QItemSelection const& selected,
                                                        QItemSelection const& deselected) {
    QListView::selectionChanged(selected, deselected);

    QModelIndexList const selectedIndices = selected.indexes();

    if (selectedIndices.empty()) {
        emit StructureArtifactChanged(nullptr);
        return;
    }

    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection size");

    QModelIndex const selectedIndex = selectedIndices.at(0);

    auto* structureArtifact = selectedIndex.data(StructureArtifactsModel::POINTER).value<StructureArtifact*>();

    emit StructureArtifactChanged(structureArtifact);
}
