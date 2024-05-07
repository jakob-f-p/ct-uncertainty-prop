#include "CtStructureView.h"

#include "CtStructureDialog.h"
#include "CtStructureTreeModel.h"
#include "../CtStructureTree.h"
#include "../../Utils/Overload.h"

#include <QComboBox>
#include <QDialog>
#include <QEvent>

CtStructureView::CtStructureView(CtStructureTree& ctStructureTree) {
    setModel(new CtStructureTreeModel(ctStructureTree));
    setItemDelegate(new CtStructureDelegate());
}

CtStructureView::CtStructureDelegate::CtStructureDelegate(QObject* parent) : DialogDelegate(parent) {}

auto CtStructureView::CtStructureDelegate::getDialog(QModelIndex const& modelIndex,
                                                     QWidget* parent) const noexcept -> QDialog* {
    auto const& dataVariant = modelIndex.data(Qt::UserRole).value<StructureDataVariant>();

    CtStructureDialog* dialog = std::visit(Overload {
            [=](CombinedStructureData const&) -> CtStructureDialog* { return new CombinedStructureDialog(CtStructureDialog::DialogMode::EDIT, parent); },
            [=](BasicStructureData const&) -> CtStructureDialog*    { return new BasicStructureDialog(CtStructureDialog::DialogMode::EDIT, parent); },
    }, dataVariant);

    return dialog;
}

void CtStructureView::CtStructureDelegate::setEditorData(QWidget* editor, QModelIndex const& index) const {
    auto const& dataVariant = index.data(Qt::UserRole).value<StructureDataVariant>();

    std::visit(Overload {
        [editor](BasicStructureData const& data)    { SetWidgetData<BasicStructureWidget>(editor, data); },
        [editor](CombinedStructureData const& data) { SetWidgetData<CombinedStructureWidget>(editor, data); }
    }, dataVariant);
}

void CtStructureView::CtStructureDelegate::setModelData(QWidget* editor,
                                                        QAbstractItemModel* model,
                                                        QModelIndex const& index) const {
    QVariant const editedData = index.data(TreeModelRoles::IS_BASIC_STRUCTURE).toBool()
            ? QVariant::fromValue(GetWidgetData<BasicStructureWidget>(editor))
            : QVariant::fromValue(GetWidgetData<CombinedStructureWidget>(editor));

    model->setData(index, editedData);
}
