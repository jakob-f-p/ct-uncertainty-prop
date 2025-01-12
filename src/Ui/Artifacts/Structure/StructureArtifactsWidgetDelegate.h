#pragma once

#include <QStyledItemDelegate>

class Pipeline;

class StructureArtifactsWidgetDelegate : public QStyledItemDelegate {
public:
    explicit StructureArtifactsWidgetDelegate(Pipeline const& pipeline, QWidget* parent = nullptr);

    auto createEditor(QWidget* parent, QStyleOptionViewItem const& option, QModelIndex const& index) const -> QWidget* override;

    void setEditorData(QWidget* editor, QModelIndex const& index) const override {}

    void setModelData(QWidget* editor, QAbstractItemModel* model, QModelIndex const& index) const override {}

    void updateEditorGeometry(QWidget* editor,
                              QStyleOptionViewItem const& option,
                              QModelIndex const& index) const override {}

protected Q_SLOTS:
    void Close();

protected:
    auto eventFilter(QObject* object, QEvent* event) -> bool override { return false; }


    Pipeline const& APipeline;
};
