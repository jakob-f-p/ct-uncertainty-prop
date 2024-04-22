#include "CtStructureDelegate.h"

#include "CtStructureDialog.h"
#include "CtStructureTreeModel.h"
#include "../CtStructureTree.h"

#include <QComboBox>
#include <QDialog>
#include <QEvent>

void DialogDelegate::updateEditorGeometry(QWidget* editor,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {}

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

bool DialogDelegate::eventFilter(QObject* object, QEvent* event) { return false; }

CtStructureDelegate::CtStructureDelegate(QObject* parent) : DialogDelegate(parent) {}

auto CtStructureDelegate::createEditor(QWidget* parent,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const -> QWidget* {
    if (!index.isValid())
        return nullptr;

    const auto& dataVariant = index.data(Qt::UserRole).value<StructureDataVariant>();
    CtStructureDialog* dialog = std::visit(Overload{
        [](const CombinedStructureData&) -> CtStructureDialog* { return new CombinedStructureDialog(CtStructureDialog::DialogMode::EDIT); },
        [](const BasicStructureData&) -> CtStructureDialog*    { return new BasicStructureDialog(CtStructureDialog::DialogMode::EDIT); },
    }, dataVariant);

    connect(dialog, &CtStructureDialog::accepted, this, &CtStructureDelegate::commitEdit);
    connect(dialog, &CtStructureDialog::rejected, this, &CtStructureDelegate::discardChanges);

    return dialog;
}

void CtStructureDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    const auto& dataVariant = index.data(Qt::UserRole).value<StructureDataVariant>();

    std::visit(Overload{
        [editor](const BasicStructureData& data)    { BasicStructureWidget::SetWidgetData(editor, data); },
        [editor](const CombinedStructureData& data) { CombinedStructureWidget::SetWidgetData(editor, data); }
    }, dataVariant);
}

void CtStructureDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    const QVariant editedData = index.data(TreeModelRoles::IS_BASIC_STRUCTURE).toBool()
            ? QVariant::fromValue(BasicStructureWidget::GetWidgetData(editor))
            : QVariant::fromValue(CombinedStructureWidget::GetWidgetData(editor));

    model->setData(index, editedData);
}
