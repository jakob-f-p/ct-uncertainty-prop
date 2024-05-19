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
    explicit CtStructureDialog(QWidget* structureWidget, DialogMode mode, QWidget* parent = nullptr);

    QVBoxLayout* Layout;
};

class BasicStructureDialog : public CtStructureDialog {
public:
    explicit BasicStructureDialog(DialogMode mode, QWidget* parent = nullptr);
};

class CombinedStructureDialog : public CtStructureDialog {
public:
    explicit CombinedStructureDialog(DialogMode mode, QWidget* parent = nullptr);
};



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
