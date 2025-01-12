#include "StructureArtifactsWidgetDelegate.h"

#include "StructureArtifactsWidgetDialog.h"
#include "../../../Artifacts/Pipeline.h"
#include "../../../Utils/IndexTypes.h"

StructureArtifactsWidgetDelegate::StructureArtifactsWidgetDelegate(Pipeline const& pipeline, QWidget* parent) :
        QStyledItemDelegate(parent),
        APipeline(pipeline) {
}

auto StructureArtifactsWidgetDelegate::createEditor(QWidget* parent,
                                                    QStyleOptionViewItem const& /*option*/,
                                                    QModelIndex const& index) const -> QWidget* {
    if (!index.isValid())
        return nullptr;

    auto const structureIdx = idx_t::FromSigned(index.internalId());
    if (!structureIdx)
        throw std::runtime_error("Invalid internal id");

    auto& structureArtifactList = APipeline.GetStructureArtifactList(*structureIdx);
    auto const title = index.data().toString().toStdString();
    auto* dialog = new StructureArtifactsWidgetDialog(structureArtifactList, title, parent);

    connect(dialog, &QDialog::rejected, this, &StructureArtifactsWidgetDelegate::Close);

    return dialog;
}

void StructureArtifactsWidgetDelegate::Close() {
    auto* ctStructureEditDialog = qobject_cast<QDialog*>(sender());

    Q_EMIT closeEditor(ctStructureEditDialog);
}
