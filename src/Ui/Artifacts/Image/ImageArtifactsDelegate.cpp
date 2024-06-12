#include "ImageArtifactsDelegate.h"

#include "../ArtifactsDialog.h"
#include "../../../Artifacts/Image/ImageArtifact.h"

auto ImageArtifactsDelegate::getDialog(QModelIndex const& /*index*/, QWidget* parent) const noexcept -> QDialog* {
    return new ImageArtifactDialog(ArtifactsDialog::Mode::EDIT, parent);
}

void ImageArtifactsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto data = index.data(Qt::UserRole).value<ImageArtifactData>();

    ImageArtifactWidget::SetWidgetData(editor, data);
}

void ImageArtifactsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto data = ImageArtifactWidget::GetWidgetData(editor);

    model->setData(index, QVariant::fromValue(data));
}
