#include "StructureArtifactsView.h"

#include "ArtifactsDialog.h"
#include "StructureArtifactsModel.h"
#include "../Structure/StructureArtifact.h"

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
