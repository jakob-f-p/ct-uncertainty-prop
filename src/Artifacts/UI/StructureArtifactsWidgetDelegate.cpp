#include "StructureArtifactsWidgetDelegate.h"

#include "StructureArtifactsWidgetDialog.h"
#include "../Pipeline.h"
#include "../../Modeling/CtStructure.h"

StructureArtifactsWidgetDelegate::StructureArtifactsWidgetDelegate(const Pipeline& pipeline, QWidget* parent) :
        QStyledItemDelegate(parent),
        APipeline(pipeline) {
}

QWidget* StructureArtifactsWidgetDelegate::createEditor(QWidget* parent,
                                                        const QStyleOptionViewItem& option,
                                                        const QModelIndex& index) const {
    if (!index.isValid())
        return nullptr;

    auto* structure = static_cast<CtStructure*>(index.internalPointer());
    if (!structure)
        return nullptr;

    auto& structureWrapper = APipeline.GetArtifactStructureWrapper(*structure);
    auto* dialog = new StructureArtifactsWidgetDialog(structureWrapper, parent);

    return dialog;
}

void StructureArtifactsWidgetDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {}

void StructureArtifactsWidgetDelegate::setModelData(QWidget* editor,
                                                    QAbstractItemModel* model,
                                                    const QModelIndex& index) const {}

void StructureArtifactsWidgetDelegate::updateEditorGeometry(QWidget* editor,
                                                            const QStyleOptionViewItem& option,
                                                            const QModelIndex& index) const {}

bool StructureArtifactsWidgetDelegate::eventFilter(QObject* object, QEvent* event) {
    return false;
}
