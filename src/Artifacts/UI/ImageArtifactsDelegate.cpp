#include "ImageArtifactsDelegate.h"
#include "../ImageArtifactDetails.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>

QString ImageArtifactsDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    if (value.canConvert<ImageArtifactDetails>()) {
        return value.value<ImageArtifactDetails>().ViewName;
    }

    return QStyledItemDelegate::displayText(value, locale);
}

QWidget* ImageArtifactsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const {
    if (!index.isValid()) {
        return nullptr;
    }

    auto* imageArtifact = static_cast<ImageArtifact*>(index.internalPointer());

    auto* editDialog = new QDialog(parent);
    editDialog->setMinimumSize(200, 100);
    editDialog->setWindowTitle("Edit");

    auto* vLayout = new QVBoxLayout(editDialog);
    vLayout->setAlignment(Qt::AlignTop);
    imageArtifact->ProvideEditWidgets(vLayout);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    vLayout->addStretch();
    vLayout->addSpacing(20);
    vLayout->addWidget(dialogButtonBar);

    connect(dialogButtonBar, &QDialogButtonBox::accepted, editDialog, &QDialog::accepted);
    connect(dialogButtonBar, &QDialogButtonBox::rejected, editDialog, &QDialog::rejected);

    connect(editDialog, &QDialog::accepted, this, &ImageArtifactsDelegate::commitEdit);
    connect(editDialog, &QDialog::rejected, this, &ImageArtifactsDelegate::discardChanges);

    return editDialog;
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

    imageArtifact->SetData(details);
}

void ImageArtifactsDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const {
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
