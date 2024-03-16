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
    explicit CtStructureEditDialog(QWidget* parent = nullptr, bool autoClose = false);

    void SetImplicitCtStructureData(const ImplicitCtStructureDetails& implicitCtStructureDetails);
    void SetImplicitStructureCombinationData(const ImplicitStructureCombinationDetails& implicitStructureCombinationDetails);

    ImplicitCtStructureDetails GetImplicitCtStructureData();
    ImplicitStructureCombinationDetails GetImplicitStructureCombinationData();

    void HideImplicitCtStructureSection();
    void HideImplicitStructureCombinationSection();

private:
    static void createTransformationEditGroup(const std::string& title,
                                              std::array<QDoubleSpinBox*, 3>& transformSpinBoxes,
                                              QVBoxLayout* parentLayout);

    void SetCtStructureData(const CtStructureDetails& ctStructureDetails);
    CtStructureDetails GetCtStructureData();

    QLineEdit* NameLineEdit;

    QWidget* ImplicitCtStructureEditSection;
    QComboBox* ImplicitFunctionEditComboBox;
    QComboBox* TissueTypeEditComboBox;

    QWidget* ImplicitStructureCombinationEditSection;
    QComboBox* OperatorTypeEditComboBox;

    std::array<std::array<QDoubleSpinBox*, 3>, 3> TransformSpinBoxes;
};
