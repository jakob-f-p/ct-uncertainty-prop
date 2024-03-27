#pragma once

#include "../BasicStructure.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>

struct BasicStructureDetails;
struct CombinedStructureDetails;

class CtStructureEditDialog : public QDialog {
    Q_OBJECT

public:
    enum Mode {
        EDIT,
        CREATE
    };

    explicit CtStructureEditDialog(Mode mode,
                                   CtStructure::SubType subType,
                                   BasicStructure::ImplicitFunctionType functionType,
                                   QWidget* parent = nullptr);

    void SetBasicStructureData(const BasicStructureDetails& basicStructureDetails);
    void SetCombinedStructureData(const CombinedStructureDetails& combinedStructureDetails);

    BasicStructureDetails GetBasicStructureData();
    CombinedStructureDetails GetCombinedStructureData();

private:
    CtStructure::SubType SubType;
    BasicStructure::ImplicitFunctionType FunctionType;

    QWidget* EditWidget;
};
