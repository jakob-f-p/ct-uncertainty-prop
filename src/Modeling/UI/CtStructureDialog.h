#pragma once

#include "../BasicStructure.h"
#include "../CombinedStructure.h"

#include <QDialog>
#include <QVBoxLayout>

class CtStructureDialog : public QDialog {
public:
    enum DialogMode {
        EDIT,
        CREATE,
    };

protected:
    explicit CtStructureDialog(DialogMode mode, QWidget* parent = nullptr);

    QVBoxLayout* Layout;
};



template<typename Ui>
class SimpleCtStructureDialog : public CtStructureDialog {
public:
    explicit SimpleCtStructureDialog(DialogMode mode, QWidget* parent = nullptr);
};

using BasicStructureDialog = SimpleCtStructureDialog<BasicStructureUi>;
using CombinedStructureDialog = SimpleCtStructureDialog<CombinedStructureUi>;



class BasicAndCombinedStructureCreateDialog : public CtStructureDialog {
public:
    explicit BasicAndCombinedStructureCreateDialog(QWidget* parent = nullptr);

    [[nodiscard]] BasicStructureDataVariant GetBasicStructureData() const;
    [[nodiscard]] CombinedStructureData GetCombinedStructureData() const;

private:
    QWidget* CombinedStructureWidget;
    QWidget* BasicStructureWidget;
};
