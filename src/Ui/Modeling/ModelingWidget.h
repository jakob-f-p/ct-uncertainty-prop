#pragma once

#include <QMainWindow>

#include <vtkSmartPointer.h>

class BasicStructureData;
class CombinedStructureData;
class CtDataSource;
class CtStructureTreeModel;
class CtStructureDialog;
class CtStructureTree;
class CtStructureView;
class DataSourceWidget;
class DoubleCoordinateRowWidget;
class ImplicitCtDataSource;
class IntegerCoordinateRowWidget;
class NrrdCtDataSource;
class RenderWidget;

class QButtonGroup;
class QItemSelection;
class QItemSelectionModel;
class QLabel;
class QPushButton;
class QStackedWidget;


class ModelingWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit ModelingWidget(CtStructureTree& ctStructureTree, QWidget* parent = nullptr);

    auto
    UpdateDataSource(CtDataSource& dataSource) -> void;

protected:
    void showEvent(QShowEvent* event) override;

private:
    auto
    SyncSpinBoxes() -> void;

    CtDataSource* CurrentDataSource;

    RenderWidget* const RenderingWidget;
    DataSourceWidget* const DataSourceWidget_;

    DoubleCoordinateRowWidget* PhysicalDimensionsSpinBoxes;
    IntegerCoordinateRowWidget* ResolutionSpinBoxes;
};


class DataSourceWidget : public QWidget {
    Q_OBJECT

public:
    explicit DataSourceWidget(CtStructureTree& ctStructureTree, CtDataSource& dataSource, QWidget* parent = nullptr);

    auto
    UpdateDataSource(CtDataSource& dataSource) -> void;

private:
    QPushButton* ImplicitButton;
    QPushButton* NrrdButton;
    QButtonGroup* SelectSourceTypeButtonGroup;

    QStackedWidget* StackedWidget;
};


class CtStructureTreeWidget : public QWidget {
    Q_OBJECT

public:
    explicit CtStructureTreeWidget(CtStructureTree& ctStructureTree, QWidget* parent = nullptr);
    ~CtStructureTreeWidget() override;

    auto
    DisableButtons() -> void;

    using OnAcceptedFunction = std::function<void(BasicStructureData const&, CombinedStructureData const&)>;

    auto
    OpenBasicAndCombinedStructureCreateDialog(OnAcceptedFunction const& onAccepted) -> void;

    auto
    UpdateButtonStates(QItemSelection const& selected, QItemSelection const&) -> void;

    auto
    GetCtDataSource() -> CtDataSource&;

    auto
    UpdateDataSource(ImplicitCtDataSource& dataSource) -> void;

private:
    vtkSmartPointer<ImplicitCtDataSource> DataSource;

    QPushButton* const AddStructureButton;
    QPushButton* const CombineWithStructureButton;
    QPushButton* const RefineWithStructureButton;
    QPushButton* const RemoveStructureButton;
    std::array<QPushButton* const, 4> const CtStructureButtons;

    CtStructureView* const TreeView;
    CtStructureTreeModel* const TreeModel;
    QItemSelectionModel* const SelectionModel;

    CtStructureDialog* CtStructureCreateDialog;
};

class CtDataImportWidget : public QWidget {
    Q_OBJECT

public:
    explicit CtDataImportWidget(QWidget* parent = nullptr);
    ~CtDataImportWidget() override;

    [[nodiscard]] auto
    Prepare() -> bool;

    auto
    GetCtDataSource() -> CtDataSource&;

    auto
    UpdateDataSource(NrrdCtDataSource& dataSource) -> void;

private:
    vtkSmartPointer<NrrdCtDataSource> DataSource;

    QLabel* const ImportNotice;
};
