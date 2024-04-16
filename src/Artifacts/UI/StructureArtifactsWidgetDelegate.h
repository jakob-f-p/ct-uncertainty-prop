#pragma once

#include <QStyledItemDelegate>

class Pipeline;

class StructureArtifactsWidgetDelegate : public QStyledItemDelegate {
public:
    explicit StructureArtifactsWidgetDelegate(const Pipeline& pipeline, QWidget* parent = nullptr);

    auto createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const -> QWidget* override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
protected:
    auto eventFilter(QObject *object, QEvent *event) -> bool override;

    const Pipeline& APipeline;
};
