#pragma once

#include "ImplicitCtStructure.h"
#include "ImplicitStructureCombination.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>

class CtStructureEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit CtStructureEditDialog(QWidget* parent = nullptr);

    void SetImplicitCtStructureData(const ImplicitCtStructureDetails& implicitCtStructureDetails);
    void SetImplicitStructureCombinationData(const ImplicitStructureCombinationDetails& implicitStructureCombinationDetails);

    ImplicitCtStructureDetails GetImplicitCtStructureData();
    ImplicitStructureCombinationDetails GetImplicitStructureCombinationData();

private:
    static void createTransformationEditGroup(const std::string& title,
                                              std::array<QDoubleSpinBox*, 3>& transformSpinBoxes,
                                              QVBoxLayout* parentLayout);

    void SetCtStructureData(const CtStructureDetails& ctStructureDetails);
    CtStructureDetails GetCtStructureData();

    QLineEdit* NameEditLineEdit;

    QWidget* ImplicitCtStructureEditArea;
    QComboBox* ImplicitFunctionEditComboBox;
    QComboBox* TissueTypeEditComboBox;

    QWidget* ImplicitStructureCombinationEditArea;
    QComboBox* OperatorTypeEditComboBox;

    std::array<std::array<QDoubleSpinBox*, 3>, 3> TransformSpinBoxes;
};
