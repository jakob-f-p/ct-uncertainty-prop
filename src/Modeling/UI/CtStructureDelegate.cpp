#include "CtStructureDelegate.h"

#include "CtStructureDialog.h"
#include "CtStructureTreeModel.h"
#include "../CtStructureTree.h"

#include <QComboBox>
#include <QDialog>
#include <QEvent>

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

    const auto& dataVariant = index.data(Qt::UserRole).value<StructureDataVariant>();
    CtStructureDialog* dialog = std::visit(Overload{
        [](const CombinedStructureData&) -> CtStructureDialog* { return new CombinedStructureDialog(CtStructureDialog::EDIT); },
        [](const auto&) -> CtStructureDialog* { return new BasicStructureDialog(CtStructureDialog::EDIT); },
    }, dataVariant);

    connect(dialog, &CtStructureDialog::accepted, this, &CtStructureDelegate::commitEdit);
    connect(dialog, &CtStructureDialog::rejected, this, &CtStructureDelegate::discardChanges);

    return dialog;
}

void CtStructureDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    const auto& dataVariant = index.data(Qt::UserRole).value<StructureDataVariant>();

    std::visit([&](auto& data) { data.PopulateWidget(editor); }, dataVariant);
}

void CtStructureDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    const QVariant editedData = index.data(TreeModelRoles::IS_BASIC_STRUCTURE).toBool()
            ? QVariant::fromValue(BasicStructureUi::GetWidgetData(editor))
            : QVariant::fromValue(CombinedStructureUi::GetWidgetData(editor));

    model->setData(index, editedData);
}
