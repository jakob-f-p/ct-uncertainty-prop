#include "CtStructureDelegate.h"

#include "CtStructureEditDialog.h"
#include "../CombinedStructure.h"

#include <QComboBox>
#include <QDialog>
#include <QEvent>
#include <QSpinBox>
#include <QLabel>

CtStructureDelegate::CtStructureDelegate(QObject* parent) :
        QStyledItemDelegate(parent) {
}

QWidget*
CtStructureDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (!index.isValid()) {
        return nullptr;
    }

    CtStructure::SubType subType = static_cast<CtStructure*>(index.internalPointer())->GetSubType();
    auto* ctStructureEditDialog = new CtStructureEditDialog(CtStructureEditDialog::EDIT, subType,
                                                            BasicStructure::ImplicitFunctionType::INVALID, parent);
    connect(ctStructureEditDialog, &CtStructureEditDialog::accepted, this, &CtStructureDelegate::commitEdit);
    connect(ctStructureEditDialog, &CtStructureEditDialog::rejected, this, &CtStructureDelegate::discardChanges);
    return ctStructureEditDialog;
}

void CtStructureDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QVariant data = index.data(Qt::EditRole);
    bool isBasicStructure = data.canConvert<BasicStructureDetails>();
    auto* dialogEditor = dynamic_cast<CtStructureEditDialog*>(editor);
    if (isBasicStructure) {
        dialogEditor->SetBasicStructureData(data.value<BasicStructureDetails>());
    } else {
        dialogEditor->SetCombinedStructureData(data.value<CombinedStructureDetails>());
    }
}

void CtStructureDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto* dialogEditor = dynamic_cast<CtStructureEditDialog*>(editor);

    QVariant editedData = static_cast<CtStructure*>(index.internalPointer())->IsBasicStructure()
            ? QVariant::fromValue(dialogEditor->GetBasicStructureData())
            : QVariant::fromValue(dialogEditor->GetCombinedStructureData());

    model->setData(index, editedData);
}

void CtStructureDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                               const QModelIndex& index) const {
}

QString CtStructureDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    if (value.canConvert<BasicStructureDetails>()) {
        return value.value<BasicStructureDetails>().ViewName;
    }

    if (value.canConvert<CombinedStructureDetails>()) {
        return value.value<CombinedStructureDetails>().ViewName;
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

bool CtStructureDelegate::eventFilter(QObject *object, QEvent *event) {
    return false;
}
