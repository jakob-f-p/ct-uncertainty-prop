#include "CtStructure.h"
#include "CtStructureEditDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>


#include <iostream>

CtStructureEditDialog::CtStructureEditDialog(QAbstractItemModel& model,
                                             QModelIndex modelIndex,
                                             QWidget* parent,
                                             Qt::WindowFlags f) : QDialog(parent, f) {
    setMinimumSize(200, 200);

    auto* structure = static_cast<CtStructure*>(modelIndex.internalPointer());
    setWindowTitle(structure->Data(CtStructure::Column::LONG_NAME).toString());

    auto* verticalLayout = new QVBoxLayout();

    auto* nameLineEdit = new QLineEdit();
    nameLineEdit->setText(structure->Data(CtStructure::Column::NAME).toString());
    verticalLayout->addWidget(nameLineEdit);

    auto* transformEditGroup = new QGroupBox("Transform");
    auto* transformVerticalLayout = new QVBoxLayout();
    createTransformationEditGroup("Translate", transformVerticalLayout);
    createTransformationEditGroup("Rotate (Angles in Degrees)", transformVerticalLayout);
    createTransformationEditGroup("Scale Factors", transformVerticalLayout);
    transformEditGroup->setLayout(transformVerticalLayout);
    verticalLayout->addWidget(transformEditGroup);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    verticalLayout->addWidget(dialogButtonBar);

    setLayout(verticalLayout);
}

QGroupBox* CtStructureEditDialog::createTransformationEditGroup(const std::string& title, QVBoxLayout* parentLayout) {
    auto* group = new QGroupBox(title.c_str());
    group->setFlat(true);
    auto* hLayout = new QHBoxLayout();
    auto* xLabel = new QLabel("x:");
    hLayout->addWidget(xLabel);
    auto* xSpinBox = new QDoubleSpinBox();
    hLayout->addWidget(xSpinBox);
    auto* yLabel = new QLabel("y:");
    hLayout->addWidget(yLabel);
    auto* ySpinBox = new QDoubleSpinBox();
    hLayout->addWidget(ySpinBox);
    auto* zLabel = new QLabel("z:");
    hLayout->addWidget(zLabel);
    auto* zSpinBox = new QDoubleSpinBox();
    hLayout->addWidget(zSpinBox);
    group->setLayout(hLayout);
    parentLayout->addWidget(group);
    return group;
}
