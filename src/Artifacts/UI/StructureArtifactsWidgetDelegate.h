#pragma once

#include <QStyledItemDelegate>

class CtStructure;
class Pipeline;

class StructureArtifactsWidgetDelegate : public QStyledItemDelegate {
public:
    explicit StructureArtifactsWidgetDelegate(const Pipeline& pipeline, QWidget* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
protected:
    bool eventFilter(QObject *object, QEvent *event) override;

    const Pipeline& APipeline;
};
