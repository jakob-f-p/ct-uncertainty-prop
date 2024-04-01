#pragma once

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>

class BasicStructureUi;
class CombinedStructureUi;

struct BasicStructureData;
struct CombinedStructureData;

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

typedef SimpleCtStructureDialog<BasicStructureUi> BasicStructureDialog;
typedef SimpleCtStructureDialog<CombinedStructureUi> CombinedStructureDialog;

class BasicAndCombinedStructureCreateDialog : public CtStructureDialog {
public:
    explicit BasicAndCombinedStructureCreateDialog(QWidget* parent = nullptr);

    void SetData(const BasicStructureData& basicStructureData, const CombinedStructureData& combinedStructureData);

    BasicStructureData GetBasicStructureData() const;
    CombinedStructureData GetCombinedStructureData() const;

protected:
    QWidget* CombinedStructureWidget;
    QWidget* BasicStructureWidget;
};
