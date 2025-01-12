#include "StructureArtifactsView.h"

#include "StructureArtifactsModel.h"
#include "../ArtifactsDialog.h"
#include "../../Utils/ModelUtils.h"
#include "../../../Artifacts/Structure/StructureArtifact.h"

StructureArtifactsView::StructureArtifactsView(StructureArtifactList& structureArtifactList) {
    QAbstractItemView::setModel(new StructureArtifactsModel(structureArtifactList, this));
    setItemDelegate(new StructureArtifactsDelegate(this));
}

StructureArtifactsView::StructureArtifactsDelegate::StructureArtifactsDelegate(QObject* parent) :
        DialogDelegate(parent) {}

auto
StructureArtifactsView::StructureArtifactsDelegate::getDialog(QModelIndex const& index,
                                                              QWidget* parent) const noexcept -> QDialog* {
    return new StructureArtifactDialog(ArtifactsDialog::Mode::EDIT, parent);
}

void StructureArtifactsView::StructureArtifactsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto const data = index.data(Qt::UserRole).value<StructureArtifactData>();

    SetWidgetData<StructureArtifactWidget>(editor, data);
}

void StructureArtifactsView::StructureArtifactsDelegate::setModelData(QWidget* editor,
                                                                      QAbstractItemModel* model,
                                                                      const QModelIndex& index) const {
    auto const data = GetWidgetData<StructureArtifactWidget>(editor);

    model->setData(index, QVariant::fromValue(data));
}

StructureArtifactsReadOnlyView::StructureArtifactsReadOnlyView(StructureArtifactList const& structureArtifactList) :
        StructureArtifactsView(const_cast<StructureArtifactList&>(structureArtifactList)),
        Model(new StructureArtifactsReadOnlyModel(structureArtifactList, this)) {

    auto const* oldModel = model();
    QAbstractItemView::setModel(Model);
    delete oldModel;
}

auto StructureArtifactsReadOnlyView::Select(StructureArtifact const& structureArtifact) const -> void {
    auto const structureArtifactPointer = QVariant::fromValue(const_cast<StructureArtifact*>(&structureArtifact));
    auto const match = Search(*Model, StructureArtifactsModel::Roles::POINTER,
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
        Q_EMIT StructureArtifactChanged(nullptr);
        return;
    }

    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection size");

    QModelIndex const selectedIndex = selectedIndices.at(0);

    auto* structureArtifact = selectedIndex.data(StructureArtifactsModel::POINTER).value<StructureArtifact*>();

    Q_EMIT StructureArtifactChanged(structureArtifact);
}
