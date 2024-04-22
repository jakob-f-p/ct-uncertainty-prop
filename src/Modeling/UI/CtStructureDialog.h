#pragma once

#include <QDialog>

class BasicStructureWidget;
class CombinedStructureWidget;

class QVBoxLayout;

class CtStructureDialog : public QDialog {
public:
    enum struct DialogMode : uint8_t {
        EDIT,
        CREATE
    };

protected:
    explicit CtStructureDialog(DialogMode mode, QWidget* parent = nullptr);

    QVBoxLayout* Layout;
};



template<typename Widget>
class SimpleCtStructureDialog : public CtStructureDialog {
public:
    explicit SimpleCtStructureDialog(DialogMode mode, QWidget* parent = nullptr);
};

using BasicStructureDialog = SimpleCtStructureDialog<BasicStructureWidget>;
using CombinedStructureDialog = SimpleCtStructureDialog<CombinedStructureWidget>;



class BasicAndCombinedStructureCreateDialog : public CtStructureDialog {
public:
    explicit BasicAndCombinedStructureCreateDialog(QWidget* parent = nullptr);

    [[nodiscard]] auto
    GetBasicWidget() const -> BasicStructureWidget& { return *BasicWidget; }

    [[nodiscard]] auto
    GetCombinedWidget() const -> CombinedStructureWidget& {return *CombinedWidget; }

private:
    CombinedStructureWidget* CombinedWidget;
    BasicStructureWidget* BasicWidget;
};
