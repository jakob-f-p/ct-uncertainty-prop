#include "CtStructureEditDialog.h"

#include "../CombinedStructure.h"

#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QPushButton>

CtStructureEditDialog::CtStructureEditDialog(CtStructureEditDialog::Mode mode,
                                             CtStructure::SubType subType,
                                             BasicStructure::ImplicitFunctionType functionType,
                                             QWidget* parent) :
        QDialog(parent),
        SubType(subType),
        FunctionType(functionType),
        EditWidget(nullptr) {

    setMinimumSize(200, 200);

    setModal(true);

    auto* vLayout = new QVBoxLayout();

    EditWidget = subType == CtStructure::BASIC
                                ? BasicStructure::GetEditWidget(functionType)
                                : CombinedStructure::GetEditWidget();
    vLayout->addWidget(EditWidget);

    if (mode == CREATE) {
        CtStructure::SetEditWidgetData(EditWidget,
                                       { "", "",{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 } });
    }


    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    vLayout->addWidget(dialogButtonBar);
    if (mode == CREATE) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }

    setLayout(vLayout);
}

void CtStructureEditDialog::SetBasicStructureData(const BasicStructureDetails& basicStructureDetails) {
    if (SubType == CtStructure::BASIC)
        BasicStructure::SetEditWidgetData(EditWidget, basicStructureDetails);
}

void CtStructureEditDialog::SetCombinedStructureData(const CombinedStructureDetails& combinedStructureDetails) {
    if (SubType == CtStructure::COMBINED)
        CombinedStructure::SetEditWidgetData(EditWidget, combinedStructureDetails);
}

BasicStructureDetails CtStructureEditDialog::GetBasicStructureData() {
    if (SubType != CtStructure::BASIC)
        return {};

    return BasicStructure::GetEditWidgetData(EditWidget);
}

CombinedStructureDetails CtStructureEditDialog::GetCombinedStructureData() {
    if (SubType != CtStructure::COMBINED)
        return {};

    return CombinedStructure::GetEditWidgetData(EditWidget);
}
