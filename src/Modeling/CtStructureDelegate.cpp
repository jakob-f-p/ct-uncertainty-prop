#include "CtStructureDelegate.h"
#include "CtStructure.h"
#include "ImplicitCtStructure.h"
#include "ImplicitStructureCombination.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

CtStructureDelegate::CtStructureDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

QWidget*
CtStructureDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (!index.isValid()) {
        return nullptr;
    }

    auto* dialog = new QDialog(parent);
    dialog->setModal(true);

    auto* verticalLayout = new QVBoxLayout();

    auto* nameEditBar = new QWidget();
    nameEditBar->setObjectName("NameEditBar");
    auto* nameEditLayout = new QHBoxLayout(nameEditBar);
    auto* nameLineEditLabel = new QLabel("Name");
    auto* nameLineEdit = new QLineEdit();
    nameLineEdit->setObjectName("Name");
    nameEditLayout->addWidget(nameLineEditLabel);
    nameEditLayout->addWidget(nameLineEdit);
    verticalLayout->addWidget(nameEditBar);

    auto* implicitFunctionTissueEditBar = new QWidget();
    implicitFunctionTissueEditBar->setObjectName("ImplicitCtStructureEditArea");
    auto* implicitFunctionTissueLayout = new QHBoxLayout(implicitFunctionTissueEditBar);
    auto* implicitFunctionLabel = new QLabel("Structure Type");
    implicitFunctionTissueLayout->addWidget(implicitFunctionLabel);
    auto* implicitFunctionComboBox = new QComboBox();
    implicitFunctionComboBox->setObjectName("ImplicitFunctionEdit");
    for (int i = 0; i < ImplicitCtStructure::ImplicitFunctionType::NUMBER_OF_IMPLICIT_FUNCTION_TYPES; ++i) {
        implicitFunctionComboBox->addItem(
                ImplicitCtStructure::ImplicitFunctionTypeToString(static_cast<ImplicitCtStructure::ImplicitFunctionType>(i)).c_str(),
                i
        );
    }
    implicitFunctionTissueLayout->addWidget(implicitFunctionComboBox);
    implicitFunctionTissueLayout->addSpacing(10);
    auto* tissueTypeLabel = new QLabel("Tissue Type");
    implicitFunctionTissueLayout->addWidget(tissueTypeLabel);
    auto* tissueTypeComboBox = new QComboBox();
    tissueTypeComboBox->setObjectName("TissueTypeEdit");
    tissueTypeComboBox->addItems(ImplicitCtStructure::GetTissueAndMaterialTypeNamesQ());
    implicitFunctionTissueLayout->addWidget(tissueTypeComboBox);
    verticalLayout->addWidget(implicitFunctionTissueEditBar);

    auto* operatorTypeEditBar = new QWidget();
    operatorTypeEditBar->setObjectName("ImplicitStructureCombinationEditArea");
    auto* operatorTypeEditLayout = new QHBoxLayout(operatorTypeEditBar);
    auto* operatorTypeLabel = new QLabel("Operator Type");
    operatorTypeEditLayout->addWidget(operatorTypeLabel);
    auto* operatorTypeComboBox = new QComboBox();
    operatorTypeComboBox->setObjectName("OperatorTypeEdit");
    for (int i = 0; i < ImplicitStructureCombination::OperatorType::NUMBER_OF_OPERATOR_TYPES; ++i) {
        operatorTypeComboBox->addItem(
                ImplicitStructureCombination::OperatorTypeToString(static_cast<ImplicitStructureCombination::OperatorType>(i)).c_str(),
                i
                );
    }
    operatorTypeEditLayout->addWidget(operatorTypeComboBox);
    verticalLayout->addWidget(operatorTypeEditBar);

    auto* transformEditGroup = new QGroupBox("Transform");
    auto* transformVerticalLayout = new QVBoxLayout(transformEditGroup);
    createTransformationEditGroup("Translate", transformVerticalLayout);
    createTransformationEditGroup("Rotate", transformVerticalLayout);
    createTransformationEditGroup("Scale", transformVerticalLayout);
    verticalLayout->addWidget(transformEditGroup);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(dialogButtonBar, &QDialogButtonBox::accepted, dialog, &QDialog::close);
    verticalLayout->addWidget(dialogButtonBar);

    dialog->setLayout(verticalLayout);
    dialog->setMinimumSize(200, 200);

    return dialog;
}

void CtStructureDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QVariant data = index.data(Qt::EditRole);
    QVariantMap map = data.toMap();

    bool isCombination = map.contains(CtStructure::DataKeyToString(CtStructure::OPERATOR_TYPE).c_str());
    QWidget* widgetToHide = isCombination
            ? editor->findChild<QWidget*>("ImplicitCtStructureEditArea")
            : editor->findChild<QWidget*>("ImplicitStructureCombinationEditArea");
    widgetToHide->hide();

    QString name = map[CtStructure::DataKeyToString(CtStructure::NAME).c_str()].toString();
    editor->findChild<QLineEdit*>("Name")->setText(name);

    if (isCombination) {
        auto* operatorTypeComboBox = editor->findChild<QComboBox*>("OperatorTypeEdit");
        int operatorType = map[CtStructure::DataKeyToString(CtStructure::OPERATOR_TYPE).c_str()].toInt();
        int idx = operatorTypeComboBox->findData(operatorType);
        if (idx != -1) {
            operatorTypeComboBox->setCurrentIndex(idx);
        }
    } else {
        auto* implicitFunctionComboBox = editor->findChild<QComboBox*>("ImplicitFunctionEdit");
        int implicitFunctionType = map[CtStructure::DataKeyToString(CtStructure::IMPLICIT_FUNCTION_TYPE).c_str()].toInt();
        int idx = implicitFunctionComboBox->findData(implicitFunctionType);
        if (idx != -1) {
            implicitFunctionComboBox->setCurrentIndex(idx);
        }

        auto* tissueTypeComboBox = editor->findChild<QComboBox*>("TissueTypeEdit");
        QString tissueType = map[CtStructure::DataKeyToString(CtStructure::IMPLICIT_FUNCTION_TYPE).c_str()].toString();
        int idx2 = implicitFunctionComboBox->findText(tissueType);
        if (idx2 != -1) {
            tissueTypeComboBox->setCurrentIndex(idx2);
        }
    }

    QVariantList transform = map[CtStructure::DataKeyToString(CtStructure::TRANSFORM).c_str()].toList();
    std::vector<std::string> transformNames{ "Translate", "Rotate", "Scale" };
    std::vector<std::string> axisNames{ "x", "y", "z" };
    QDoubleSpinBox* spinBox;
    QVariantList floatList;
    float val;
    std::string spinBoxName;
    for (int i = 0; i < 3; ++i) {
        floatList = transform.at(i).toList();
        for (int j = 0; j < 3; ++j) {
            spinBoxName = axisNames[j] + transformNames[i];
            spinBox = editor->findChild<QDoubleSpinBox*>(spinBoxName.c_str());
            val = floatList.at(j).toFloat();
            spinBox->setValue(val);
        }
    }
}

void CtStructureDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QVariantMap editorData;

    auto* nameEdit = editor->findChild<QLineEdit*>("Name");
    editorData[CtStructure::DataKeyToString(CtStructure::DataKey::NAME).c_str()] = nameEdit->text();

    if (editor->findChild<QWidget*>("ImplicitStructureCombinationEditArea")->isVisible()) {
        auto* operatorTypeComboBox = editor->findChild<QComboBox*>("OperatorTypeEdit");
        int operatorType = operatorTypeComboBox->currentData().toInt();
        editorData[CtStructure::DataKeyToString(CtStructure::DataKey::OPERATOR_TYPE).c_str()] = operatorType;
    }

    if (editor->findChild<QWidget*>("ImplicitCtStructureEditArea")->isVisible()) {
        auto* implicitFunctionComboBox = editor->findChild<QComboBox*>("ImplicitFunctionEdit");
        int implicitFunctionType = implicitFunctionComboBox->currentData().toInt();
        editorData[CtStructure::DataKeyToString(CtStructure::DataKey::IMPLICIT_FUNCTION_TYPE).c_str()] = implicitFunctionType;

        auto* tissueTypeComboBox = editor->findChild<QComboBox*>("TissueTypeEdit");
        QString tissueType = tissueTypeComboBox->currentText();
        editorData[CtStructure::DataKeyToString(CtStructure::DataKey::TISSUE_TYPE).c_str()] = tissueType;
    }

    QVariantList transformData(3);
    std::vector<std::string> transformNames{ "Translate", "Rotate", "Scale" };
    std::vector<std::string> axisNames{ "x", "y", "z" };
    QDoubleSpinBox* spinBox;
    float val;
    std::string spinBoxName;
    for (int i = 0; i < 3; ++i) {
        QVariantList floatList(3);
        for (int j = 0; j < 3; ++j) {
            spinBoxName = axisNames[j] + transformNames[i];
            spinBox = editor->findChild<QDoubleSpinBox*>(spinBoxName.c_str());
            val = static_cast<float>(spinBox->value());
            floatList[j] = val;
        }
        transformData[i] = floatList;
    }

    model->setData(index, editorData);
}

void CtStructureDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                               const QModelIndex& index) const {
}

QString CtStructureDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    if (value.canConvert<QVariantMap>()) {
        QVariant val = QVariant(value);
        QVariantMap map = val.toMap();
        std::string nameKey = CtStructure::DataKeyToString(CtStructure::TREE_VIEW_NAME);
        if (map.contains(nameKey.c_str())) {
            return QStyledItemDelegate::displayText(map[nameKey.c_str()], locale);
        }
    }
    return QStyledItemDelegate::displayText(value, locale);
}

void CtStructureDelegate::createTransformationEditGroup(const std::string& title, QVBoxLayout* parentLayout) const {
    auto* bar = new QWidget();
    bar->setObjectName(title);

    auto* hLayout = new QHBoxLayout(bar);

    auto* titleLabel = new QLabel(title.c_str());
    hLayout->addWidget(titleLabel);
    hLayout->addStretch();

    std::vector<std::string> axisNames { "x", "y", "z" };
    for (const auto &axisName : axisNames) {
        auto* label = new QLabel(axisName.c_str());
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hLayout->addWidget(label);
        auto* spinBox = new QDoubleSpinBox();
        spinBox->setRange(-100.0, 100.0);
        spinBox->setObjectName(axisName + title);
        hLayout->addWidget(spinBox);
    }

    parentLayout->addWidget(bar);
}

void CtStructureDelegate::acceptDialog(QDialog* dialog) {
//    emit QDialogButtonBox::ac
}

void CtStructureDelegate::closeEditor() {

}
