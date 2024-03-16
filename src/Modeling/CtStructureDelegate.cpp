#include "CtStructureDelegate.h"
#include "ImplicitCtStructure.h"
#include "ImplicitStructureCombination.h"
#include "CtStructureEditDialog.h"

#include <QComboBox>
#include <QDialog>
#include <QSpinBox>
#include <QLabel>

CtStructureDelegate::CtStructureDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

QWidget*
CtStructureDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (!index.isValid()) {
        return nullptr;
    }

    auto* ctStructureEditDialog = new CtStructureEditDialog(parent);
    connect(ctStructureEditDialog, &CtStructureEditDialog::accepted, this, &CtStructureDelegate::commitEdit);
    connect(ctStructureEditDialog, &CtStructureEditDialog::rejected, this, &CtStructureDelegate::discardChanges);
    return ctStructureEditDialog;
}

void CtStructureDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QVariant data = index.data(Qt::EditRole);
    bool isImplicitCtStructure = data.canConvert<ImplicitCtStructureDetails>();
    auto* dialogEditor = dynamic_cast<CtStructureEditDialog*>(editor);
    if (isImplicitCtStructure) {
        dialogEditor->SetImplicitCtStructureData(data.value<ImplicitCtStructureDetails>());
    } else {
        dialogEditor->SetImplicitStructureCombinationData(data.value<ImplicitStructureCombinationDetails>());
    }
}

void CtStructureDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto *dialogEditor = dynamic_cast<CtStructureEditDialog *>(editor);

    bool isImplicitCtStructure = index.data(Qt::UserRole).toBool();
    QVariant editedData = isImplicitCtStructure
            ? QVariant::fromValue(dialogEditor->GetImplicitCtStructureData())
            : QVariant::fromValue(dialogEditor->GetImplicitStructureCombinationData());

    model->setData(index, editedData);
}

void CtStructureDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                               const QModelIndex& index) const {
}

QString CtStructureDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    if (value.canConvert<ImplicitCtStructureDetails>()) {
        return value.value<ImplicitCtStructureDetails>().ViewName;
    }

    if (value.canConvert<ImplicitStructureCombinationDetails>()) {
        return value.value<ImplicitStructureCombinationDetails>().ViewName;
    }

    return QStyledItemDelegate::displayText(value, locale);
}

void CtStructureDelegate::commitEdit() {
    auto* ctStructureEditDialog = qobject_cast<CtStructureEditDialog*>(sender());
    emit commitData(ctStructureEditDialog);
    emit closeEditor(ctStructureEditDialog);
}

void CtStructureDelegate::discardChanges() {
    auto* ctStructureEditDialog = qobject_cast<CtStructureEditDialog*>(sender());
    emit closeEditor(ctStructureEditDialog);
}
