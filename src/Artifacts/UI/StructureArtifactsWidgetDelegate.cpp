#include "StructureArtifactsWidgetDelegate.h"

#include "StructureArtifactsWidgetDialog.h"
#include "../Pipeline.h"
#include "../../Modeling/CtStructure.h"

StructureArtifactsWidgetDelegate::StructureArtifactsWidgetDelegate(const Pipeline& pipeline, QWidget* parent) :
        QStyledItemDelegate(parent),
        APipeline(pipeline) {
}

auto StructureArtifactsWidgetDelegate::createEditor(QWidget* parent,
                                                    const QStyleOptionViewItem& option,
                                                    const QModelIndex& index) const -> QWidget* {
    if (!index.isValid())
        return nullptr;

    const auto structureIdx = static_cast<idx_t>(index.internalId());
    if (structureIdx < 0)
        throw std::runtime_error("Invalid internal id");

    auto& structureWrapper = APipeline.GetStructureArtifactListCollectionForIdx(structureIdx);
    auto title = index.data().toString().toStdString();
    auto* dialog = new StructureArtifactsWidgetDialog(structureWrapper, title, parent);

    return dialog;
}

void StructureArtifactsWidgetDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {}

void StructureArtifactsWidgetDelegate::setModelData(QWidget* editor,
                                                    QAbstractItemModel* model,
                                                    const QModelIndex& index) const {}

void StructureArtifactsWidgetDelegate::updateEditorGeometry(QWidget* editor,
                                                            const QStyleOptionViewItem& option,
                                                            const QModelIndex& index) const {}

auto StructureArtifactsWidgetDelegate::eventFilter(QObject* object, QEvent* event) -> bool {
    return false;
}
