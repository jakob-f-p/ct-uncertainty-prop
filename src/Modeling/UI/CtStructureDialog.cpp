#include "CtStructureDialog.h"

#include "../BasicStructure.h"
#include "../CombinedStructure.h"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

CtStructureDialog::CtStructureDialog(DialogMode mode, QWidget* parent) :
        QDialog(parent),
        Layout(new QVBoxLayout(this)) {

    setMinimumSize(800, 800);
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    setModal(true);

    setWindowTitle(mode == DialogMode::CREATE ? "Create" : "Edit");

    Layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);

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

template class SimpleCtStructureDialog<BasicStructureWidget>;
template class SimpleCtStructureDialog<CombinedStructureWidget>;

template<typename Widget>
SimpleCtStructureDialog<Widget>::SimpleCtStructureDialog(CtStructureDialog::DialogMode mode, QWidget* parent) :
        CtStructureDialog(mode, parent) {
    Layout->insertWidget(0, new Widget());
}

BasicAndCombinedStructureCreateDialog::BasicAndCombinedStructureCreateDialog(QWidget* parent) :
        CtStructureDialog(DialogMode::CREATE, parent),
        CombinedWidget(new CombinedStructureWidget()),
        BasicWidget(new BasicStructureWidget()) {

    auto* tabWidget = new QTabWidget();
    tabWidget->addTab(CombinedWidget, "Combination");
    tabWidget->addTab(BasicWidget, "Basic");
    Layout->insertWidget(0, tabWidget);
}
