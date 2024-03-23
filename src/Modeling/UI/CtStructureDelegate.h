#pragma once

#include <QGroupBox>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

class CtStructureDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit CtStructureDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    void
    updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    QString displayText(const QVariant& value, const QLocale& locale) const override;

public slots:
    void commitEdit();
    void discardChanges();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

