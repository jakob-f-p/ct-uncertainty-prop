#include "ImageArtifactsDelegate.h"

#include "ArtifactsEditDialog.h"
#include "../ImageArtifactDetails.h"

#include <QDialog>
#include <QLayout>

QWidget* ImageArtifactsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    auto* imageArtifact = static_cast<ImageArtifact*>(index.internalPointer());

    auto* dialog =  new ArtifactsEditDialog(ArtifactsEditDialog::EDIT, parent, imageArtifact);

    connect(dialog, &ArtifactsEditDialog::accepted, this, &ImageArtifactsDelegate::commitEdit);
    connect(dialog, &ArtifactsEditDialog::rejected, this, &ImageArtifactsDelegate::discardChanges);

    return dialog;
}

void ImageArtifactsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* imageArtifact = static_cast<ImageArtifact*>(index.internalPointer());
    auto* dialogEditor = dynamic_cast<QDialog*>(editor);

    QVariant data = index.data(Qt::EditRole);
    auto details = data.value<ImageArtifactDetails>();

    imageArtifact->SetEditWidgetData(dialogEditor, details);
}

void ImageArtifactsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto* imageArtifact = static_cast<ImageArtifact*>(index.internalPointer());
    auto* dialogEditor = dynamic_cast<QDialog*>(editor);

    ImageArtifactDetails details = imageArtifact->GetImageArtifactEditWidgetData(dialogEditor);

    model->setData(index, QVariant::fromValue(details));
}

void ImageArtifactsDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const {
}

QString ImageArtifactsDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    if (value.canConvert<ImageArtifactDetails>()) {
        return value.value<ImageArtifactDetails>().ViewName;
    }

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
