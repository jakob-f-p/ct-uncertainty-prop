#pragma once

#include <QGroupBox>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

class DialogDelegate : public QStyledItemDelegate {
public:
    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

protected slots:
    void commitEdit();
    void discardChanges();

protected:
    explicit DialogDelegate(QObject* parent = nullptr);

    bool eventFilter(QObject *object, QEvent *event) override;
};

class CtStructureDelegate : public DialogDelegate {
public:
    explicit CtStructureDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

