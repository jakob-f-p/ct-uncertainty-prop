#pragma once

#include <QAbstractItemModel>
#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>

class CtStructureEditDialog : public QDialog {
public:
    explicit CtStructureEditDialog(QAbstractItemModel& model,
                                   QModelIndex modelIndex,
                                   QWidget* parent = nullptr,
                                   Qt::WindowFlags f = Qt::WindowFlags());

private:
    QGroupBox* createTransformationEditGroup(const std::string& title, QVBoxLayout* parentLayout);
};
