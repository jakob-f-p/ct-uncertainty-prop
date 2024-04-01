#include "CtStructureDelegate.h"

#include "CtStructureDialog.h"
#include "../BasicStructure.h"
#include "../CombinedStructure.h"

#include <QComboBox>
#include <QDialog>
#include <QEvent>
#include <QSpinBox>

void DialogDelegate::updateEditorGeometry(QWidget* editor,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
}

void DialogDelegate::commitEdit() {
    auto* ctStructureEditDialog = qobject_cast<QDialog*>(sender());

    emit commitData(ctStructureEditDialog);
    emit closeEditor(ctStructureEditDialog);
}

void DialogDelegate::discardChanges() {
    auto* ctStructureEditDialog = qobject_cast<QDialog*>(sender());

    emit commitData(ctStructureEditDialog);
    emit closeEditor(ctStructureEditDialog);
}

DialogDelegate::DialogDelegate(QObject* parent) :
        QStyledItemDelegate(parent) {
}

bool DialogDelegate::eventFilter(QObject* object, QEvent* event) {
    return false;
}

CtStructureDelegate::CtStructureDelegate(QObject* parent) :
        DialogDelegate(parent) {
}

QWidget* CtStructureDelegate::createEditor(QWidget* parent,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    CtStructureDialog* dialog;
    if (CtStructure::IsBasic(index.internalPointer()))
        dialog = new BasicStructureDialog(CtStructureDialog::EDIT);
    else
        dialog = new CombinedStructureDialog(CtStructureDialog::EDIT);

    connect(dialog, &CtStructureDialog::accepted, this, &CtStructureDelegate::commitEdit);
    connect(dialog, &CtStructureDialog::rejected, this, &CtStructureDelegate::discardChanges);

    return dialog;
}

void CtStructureDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QVariant data = index.data(Qt::EditRole);
    if (CtStructure::IsBasic(index.internalPointer()))
        BasicStructureUi::SetWidgetData(editor, data.value<BasicStructureData>());
    else
        CombinedStructureUi::SetWidgetData(editor, data.value<CombinedStructureData>());
}

void CtStructureDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QVariant editedData = CtStructure::IsBasic(index.internalPointer())
            ? QVariant::fromValue(BasicStructureUi::GetWidgetData(editor))
            : QVariant::fromValue(CombinedStructureUi::GetWidgetData(editor));

    model->setData(index, editedData);
}

QString CtStructureDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    if (value.canConvert<BasicStructureData>()) {
        return value.value<BasicStructureData>().ViewName;
    }

    if (value.canConvert<CombinedStructureData>()) {
        return value.value<CombinedStructureData>().ViewName;
    }

    return QStyledItemDelegate::displayText(value, locale);
}
