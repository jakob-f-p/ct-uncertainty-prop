#include "CtStructureDialog.h"

#include "../../Modeling/BasicStructure.h"
#include "../../Modeling/CombinedStructure.h"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

CtStructureDialog::CtStructureDialog(QWidget* structureWidget, DialogMode mode, QWidget* parent) :
        QDialog(parent),
        Layout(new QVBoxLayout(this)) {

    if (!structureWidget)
        throw std::runtime_error("Structure widget must not be null");

    setMinimumSize(800, 800);

    setModal(true);

    setWindowTitle(mode == DialogMode::CREATE ? "Create" : "Edit");

    Layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);

    Layout->addWidget(structureWidget);

    auto* dialogButtonBar = new QDialogButtonBox();
    dialogButtonBar->setOrientation(Qt::Horizontal);
    dialogButtonBar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    Layout->addWidget(dialogButtonBar);

    if (mode == DialogMode::CREATE) {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        connect(dialogButtonBar, &QDialogButtonBox::accepted, this, &QDialog::accepted);
        connect(dialogButtonBar, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    }
}

BasicStructureDialog::BasicStructureDialog(CtStructureDialog::DialogMode mode, QWidget* parent) :
        CtStructureDialog(new BasicStructureWidget(), mode, parent) {}

CombinedStructureDialog::CombinedStructureDialog(CtStructureDialog::DialogMode mode, QWidget* parent) :
        CtStructureDialog(new CombinedStructureWidget(), mode, parent) {}

BasicAndCombinedStructureCreateDialog::BasicAndCombinedStructureCreateDialog(QWidget* parent) :
        CtStructureDialog([]() {
                              auto* tabWidget = new QTabWidget();
                              tabWidget->addTab(new CombinedStructureWidget(), "Combination");
                              tabWidget->addTab(new BasicStructureWidget(), "Basic");
                              return tabWidget;
                          }(),
                          DialogMode::CREATE,
                          parent),
        CombinedWidget(findChild<CombinedStructureWidget*>()),
        BasicWidget(findChild<BasicStructureWidget*>()) {
}
