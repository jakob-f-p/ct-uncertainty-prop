#include "CtStructureEditDialog.h"

#include "../ImplicitCtStructure.h"
#include "../ImplicitStructureCombination.h"

#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>

CtStructureEditDialog::CtStructureEditDialog(QWidget* parent, bool autoClose) :
        QDialog(parent),
        TransformSpinBoxes {} {
    setMinimumSize(200, 200);

    setModal(true);

    auto* verticalLayout = new QVBoxLayout();

    auto* nameEditBar = new QWidget();
    auto* nameEditLayout = new QHBoxLayout(nameEditBar);
    auto* nameLineEditLabel = new QLabel("Name");
    nameLineEditLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    NameLineEdit = new QLineEdit();
    nameEditLayout->addWidget(nameLineEditLabel);
    nameEditLayout->addSpacing(20);
    nameEditLayout->addWidget(NameLineEdit);
    verticalLayout->addWidget(nameEditBar);


    ImplicitCtStructureEditSection = new QWidget();
    auto* implicitFunctionTissueLayout = new QHBoxLayout(ImplicitCtStructureEditSection);
    auto* implicitFunctionLabel = new QLabel("Structure Type");
    implicitFunctionTissueLayout->addWidget(implicitFunctionLabel);
    ImplicitFunctionEditComboBox = new QComboBox();
    for (const auto &implicitFunctionAndName : ImplicitCtStructure::GetImplicitFunctionTypeValues()) {
        ImplicitFunctionEditComboBox->addItem(implicitFunctionAndName.Name, implicitFunctionAndName.EnumValue);
    }
    implicitFunctionTissueLayout->addWidget(ImplicitFunctionEditComboBox);
    implicitFunctionTissueLayout->addSpacing(20);
    auto* tissueTypeLabel = new QLabel("Tissue Type");
    implicitFunctionTissueLayout->addWidget(tissueTypeLabel);
    TissueTypeEditComboBox = new QComboBox();
    TissueTypeEditComboBox->addItems(ImplicitCtStructure::GetTissueAndMaterialTypeNames());
    implicitFunctionTissueLayout->addWidget(TissueTypeEditComboBox);
    verticalLayout->addWidget(ImplicitCtStructureEditSection);


    ImplicitStructureCombinationEditSection = new QWidget();
    auto* operatorTypeEditLayout = new QHBoxLayout(ImplicitStructureCombinationEditSection);
    auto* operatorTypeLabel = new QLabel("Operator Type");
    operatorTypeEditLayout->addWidget(operatorTypeLabel);
    OperatorTypeEditComboBox = new QComboBox();
    for (const auto &operatorAndName : ImplicitStructureCombination::GetOperatorTypeValues()) {
        OperatorTypeEditComboBox->addItem(operatorAndName.Name, operatorAndName.EnumValue);
    }
    operatorTypeEditLayout->addWidget(OperatorTypeEditComboBox);
    verticalLayout->addWidget(ImplicitStructureCombinationEditSection);


    auto* transformEditGroup = new QGroupBox("Transform");
    auto* transformVerticalLayout = new QVBoxLayout(transformEditGroup);
    std::array<std::string, 3> transformNames { "Translate", "Rotate", "Scale" };
    std::array<double, 3> transformStepSizes { 2.0, 1.0, 0.1 };
    for (int i = 0; i < transformNames.size(); i++) {
        createTransformationEditGroup(transformNames[i], TransformSpinBoxes[i], transformVerticalLayout, transformStepSizes[i]);
    }
    verticalLayout->addWidget(transformEditGroup);


    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    verticalLayout->addWidget(dialogButtonBar);
    if (autoClose) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }

    setLayout(verticalLayout);
}

void CtStructureEditDialog::createTransformationEditGroup(const std::string& title,
                                                          std::array<QDoubleSpinBox*, 3>& transformSpinBoxes,
                                                          QVBoxLayout* parentLayout,
                                                          double stepSize) {
    auto* bar = new QWidget();
    bar->setObjectName(title);

    auto* hLayout = new QHBoxLayout(bar);

    auto* titleLabel = new QLabel(title.c_str());
    hLayout->addWidget(titleLabel);
    hLayout->addStretch();

    std::vector<std::string> axisNames { "x", "y", "z" };
    for (int i = 0; i < axisNames.size(); ++i) {
        hLayout->addSpacing(10);
        auto* label = new QLabel(axisNames[i].c_str());
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hLayout->addWidget(label);
        transformSpinBoxes[i] = new QDoubleSpinBox();
        transformSpinBoxes[i]->setRange(-100.0, 100.0);
        transformSpinBoxes[i]->setSingleStep(stepSize);
        hLayout->addWidget(transformSpinBoxes[i]);
    }

    parentLayout->addWidget(bar);
}

void CtStructureEditDialog::HideImplicitCtStructureSection() {
    ImplicitCtStructureEditSection->hide();
}

void CtStructureEditDialog::HideImplicitStructureCombinationSection() {
    ImplicitStructureCombinationEditSection->hide();
}

void CtStructureEditDialog::SetImplicitCtStructureData(const ImplicitCtStructureDetails& implicitCtStructureDetails) {
    SetCtStructureData(implicitCtStructureDetails);

    if (int idx = ImplicitFunctionEditComboBox->findData(implicitCtStructureDetails.ImplicitFunctionType);
            idx != -1) {
        ImplicitFunctionEditComboBox->setCurrentIndex(idx);
    }

    if (int idx = TissueTypeEditComboBox->findText(implicitCtStructureDetails.TissueName);
            idx != -1) {
        TissueTypeEditComboBox->setCurrentIndex(idx);
    }

    ImplicitStructureCombinationEditSection->hide();
}

void CtStructureEditDialog::SetImplicitStructureCombinationData(
        const ImplicitStructureCombinationDetails& implicitStructureCombinationDetails) {
    SetCtStructureData(implicitStructureCombinationDetails);

    if (int idx = OperatorTypeEditComboBox->findData(implicitStructureCombinationDetails.OperatorType);
            idx != -1) {
        OperatorTypeEditComboBox->setCurrentIndex(idx);
    }

    ImplicitCtStructureEditSection->hide();
}

void CtStructureEditDialog::SetCtStructureData(const CtStructureDetails& ctStructureDetails) {
    setWindowTitle(ctStructureDetails.ViewName);

    NameLineEdit->setText(ctStructureDetails.Name);

    for (int i = 0; i < ctStructureDetails.Transform.size(); ++i) {
        for (int j = 0; j < ctStructureDetails.Transform[i].size(); ++j) {
            TransformSpinBoxes[i][j]->setValue(ctStructureDetails.Transform[i][j]);
        }
    }
}

ImplicitCtStructureDetails CtStructureEditDialog::GetImplicitCtStructureData() {
    return {
        GetCtStructureData(),
        ImplicitFunctionEditComboBox->currentData().value<ImplicitCtStructure::ImplicitFunctionType>(),
        TissueTypeEditComboBox->currentText(),
    };
}

ImplicitStructureCombinationDetails CtStructureEditDialog::GetImplicitStructureCombinationData() {
    return {
        GetCtStructureData(),
        OperatorTypeEditComboBox->currentData().value<ImplicitStructureCombination::OperatorType>()
    };
}

CtStructureDetails CtStructureEditDialog::GetCtStructureData() {
    std::array<std::array<float, 3>, 3> transform {};
    for (int i = 0; i < transform.size(); ++i) {
        for (int j = 0; j < transform[i].size(); ++j) {
            transform[i][j] = static_cast<float>(TransformSpinBoxes[i][j]->value());
        }
    }

    return { NameLineEdit->text(), "", transform };
}
