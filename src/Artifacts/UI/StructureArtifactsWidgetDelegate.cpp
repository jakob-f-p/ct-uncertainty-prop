#include "StructureArtifactsWidgetDelegate.h"

#include "StructureArtifactsWidgetDialog.h"
#include "../Pipeline.h"

StructureArtifactsWidgetDelegate::StructureArtifactsWidgetDelegate(Pipeline const& pipeline, QWidget* parent) :
        QStyledItemDelegate(parent),
        APipeline(pipeline) {
}

auto StructureArtifactsWidgetDelegate::createEditor(QWidget* parent,
                                                    QStyleOptionViewItem const& option,
                                                    QModelIndex const& index) const -> QWidget* {
    if (!index.isValid())
        return nullptr;

    auto const structureIdx = idx_t::FromSigned(index.internalId());
    if (!structureIdx)
        throw std::runtime_error("Invalid internal id");

    auto& structureWrapper = APipeline.GetStructureArtifactListCollectionForIdx(*structureIdx);
    auto title = index.data().toString().toStdString();
    auto* dialog = new StructureArtifactsWidgetDialog(structureWrapper, title, parent);

    return dialog;
}
