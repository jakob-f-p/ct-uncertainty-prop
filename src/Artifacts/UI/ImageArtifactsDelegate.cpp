#include "ImageArtifactsDelegate.h"

#include "ArtifactsDialog.h"
#include "../ImageArtifact.h"

QWidget* ImageArtifactsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    auto* dialog =  new ArtifactsDialog(ArtifactsDialog::EDIT, parent);

    connect(dialog, &ArtifactsDialog::accepted, this, &ImageArtifactsDelegate::commitEdit);
    connect(dialog, &ArtifactsDialog::rejected, this, &ImageArtifactsDelegate::discardChanges);

    return dialog;
}

void ImageArtifactsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* artifact = static_cast<ImageArtifact*>(index.internalPointer());
    auto data  = ImageArtifactData::GetData(*artifact);

    ImageArtifactUi::SetWidgetData(editor, *data);
}

void ImageArtifactsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto imageArtifactData = ImageArtifactUi::GetWidgetData(editor);

    model->setData(index, QVariant::fromValue(*imageArtifactData));
}

void ImageArtifactsDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const {
}

QString ImageArtifactsDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    auto imageArtifactData = ImageArtifactData::FromQVariant(value);
    if (imageArtifactData)
        return imageArtifactData->ViewName;

    return QStyledItemDelegate::displayText(value, locale);
}

void ImageArtifactsDelegate::commitEdit() {
    auto* editDialog = qobject_cast<QDialog*>(sender());

    emit commitData(editDialog);
    emit closeEditor(editDialog);
}

void ImageArtifactsDelegate::discardChanges() {
    auto* editDialog = qobject_cast<QDialog*>(sender());

    emit closeEditor(editDialog);
}

bool ImageArtifactsDelegate::eventFilter(QObject* object, QEvent* event) {
    return false;
}
