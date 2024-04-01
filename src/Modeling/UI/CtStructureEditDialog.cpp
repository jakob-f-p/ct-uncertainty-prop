#include "CtStructureEditDialog.h"

#include "../BasicStructure.h"
#include "../CombinedStructure.h"

#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QPushButton>

CtStructureDialog::CtStructureDialog(DialogMode mode, QWidget* parent) :
        QDialog(parent) {

    setMinimumSize(800, 800);
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    setModal(true);

    setWindowTitle(mode == CREATE ? "Create" : "Edit");

    Layout = new QVBoxLayout(this);
    Layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    Layout->addWidget(dialogButtonBar);

    if (mode == CREATE) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }
}

template class SimpleCtStructureDialog<BasicStructureUi>;
template class SimpleCtStructureDialog<CombinedStructureUi>;

template<typename Ui>
SimpleCtStructureDialog<Ui>::SimpleCtStructureDialog(CtStructureDialog::DialogMode mode, QWidget* parent) :
        CtStructureDialog(mode, parent) {
    auto* widget = Ui::GetWidget();
    Layout->insertWidget(0, widget);
}

BasicAndCombinedStructureCreateDialog::BasicAndCombinedStructureCreateDialog(QWidget* parent) :
        CtStructureDialog(CREATE, parent),
        CombinedStructureWidget(CombinedStructureUi::GetWidget()),
        BasicStructureWidget(BasicStructureUi::GetWidget()) {

    auto* tabWidget = new QTabWidget();
    tabWidget->addTab(CombinedStructureWidget, "Combination");
    tabWidget->addTab(BasicStructureWidget, "Basic");
    Layout->insertWidget(0, tabWidget);
}

void BasicAndCombinedStructureCreateDialog::SetData(const BasicStructureData& basicStructureData,
                                                    const CombinedStructureData& combinedStructureData) {
    BasicStructureUi::SetWidgetData(BasicStructureWidget, basicStructureData);
    CombinedStructureUi::SetWidgetData(CombinedStructureWidget, combinedStructureData);
}

BasicStructureData BasicAndCombinedStructureCreateDialog::GetBasicStructureData() const {
    return BasicStructureUi::GetWidgetData(BasicStructureWidget);
}

CombinedStructureData BasicAndCombinedStructureCreateDialog::GetCombinedStructureData() const {
    return CombinedStructureUi::GetWidgetData(CombinedStructureWidget);
}
