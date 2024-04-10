#include "StructureArtifactsDelegate.h"

#include "ArtifactsDialog.h"
#include "../StructureArtifact.h"

QWidget* StructureArtifactsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    auto* dialog =  new StructureArtifactDialog(ArtifactsDialog::EDIT, parent);

    connect(dialog, &ArtifactsDialog::accepted, this, &StructureArtifactsDelegate::commitEdit);
    connect(dialog, &ArtifactsDialog::rejected, this, &StructureArtifactsDelegate::discardChanges);

    return dialog;
}

void StructureArtifactsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* artifact = static_cast<StructureArtifact*>(index.internalPointer());
    auto data  = StructureArtifactData::GetData(*artifact);

    StructureArtifactUi::SetWidgetData(editor, *data);
}

void StructureArtifactsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto imageArtifactData = StructureArtifactUi::GetWidgetData(editor);

    model->setData(index, StructureArtifactData::ToQVariant(*imageArtifactData));
}

void StructureArtifactsDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const {
}

void StructureArtifactsDelegate::commitEdit() {
    auto* editDialog = qobject_cast<QDialog*>(sender());

    emit commitData(editDialog);
    emit closeEditor(editDialog);
}

void StructureArtifactsDelegate::discardChanges() {
    auto* editDialog = qobject_cast<QDialog*>(sender());

    emit closeEditor(editDialog);
}

bool StructureArtifactsDelegate::eventFilter(QObject* object, QEvent* event) {
    return false;
}
