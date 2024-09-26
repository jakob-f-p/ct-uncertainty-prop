#include "CtStructureTreeModel.h"
#include "ModelingWidget.h"

#include "CtStructureDialog.h"
#include "CtStructureView.h"
#include "../Utils/CoordinateRowWidget.h"
#include "../Utils/RenderWidget.h"
#include "../../App.h"
#include "../../Modeling/BasicStructure.h"
#include "../../Modeling/CombinedStructure.h"
#include "../../Modeling/CtDataSource.h"
#include "../../Modeling/CtStructureTree.h"
#include "../../Modeling/ImplicitCtDataSource.h"
#include "../../Modeling/NrrdCtDataSource.h"
#include "../../Utils/Overload.h"

#include <QButtonGroup>
#include <QDockWidget>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QShowEvent>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTreeView>
#include <QVBoxLayout>


ModelingWidget::ModelingWidget(CtStructureTree& ctStructureTree, QWidget* parent) :
        QMainWindow(parent),
        CurrentDataSource(&App::GetInstance().GetCtDataSource()),
        RenderingWidget(new RenderWidget()),
        DataSourceWidget_(new DataSourceWidget(ctStructureTree, *CurrentDataSource)),
        PhysicalDimensionsSpinBoxes(new DoubleCoordinateRowWidget({ 1.0, 100.0, 1.0, 1.0  }, "Physical\nDimensions")),
        ResolutionSpinBoxes(new IntegerCoordinateRowWidget({ 16, 512, 1, 16 }, "Resolution")) {

    ctStructureTree.AddTreeEventCallback([&](CtStructureTreeEvent const&) { RenderingWidget->Render(); });

    setCentralWidget(RenderingWidget);

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(
            Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);
    dockWidget->setContentsMargins({});

    auto* dockWidgetContent = new QWidget();
    auto* verticalLayout = new QVBoxLayout(dockWidgetContent);
    verticalLayout->setContentsMargins({ 10, 0, 10, 10 });

    auto* volumeLabel = new QLabel("Volume settings");
    volumeLabel->setStyleSheet(GetHeader1StyleSheet());
    verticalLayout->addWidget(volumeLabel);
    verticalLayout->addSpacing(10);
    verticalLayout->addWidget(PhysicalDimensionsSpinBoxes);
    verticalLayout->addWidget(ResolutionSpinBoxes);

    auto* separator = new QFrame();
    separator->setFrameShape(QFrame::Shape::HLine);
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(separator);
    verticalLayout->addSpacing(20);

    verticalLayout->addWidget(DataSourceWidget_);

    verticalLayout->addStretch();

    dockWidget->setWidget(dockWidgetContent);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);


    connect(PhysicalDimensionsSpinBoxes, &DoubleCoordinateRowWidget::ValueChanged, this, [this]() {
        CurrentDataSource->SetVolumeDataPhysicalDimensions(PhysicalDimensionsSpinBoxes->GetRowData(0).ToFloatArray());
    });
    connect(ResolutionSpinBoxes, &IntegerCoordinateRowWidget::ValueChanged, this, [this]() {
        CurrentDataSource->SetVolumeNumberOfVoxels(ResolutionSpinBoxes->GetRowData(0).ToArray());
    });

    UpdateDataSource(*CurrentDataSource);
}

auto ModelingWidget::UpdateDataSource(CtDataSource& dataSource) -> void {
    CurrentDataSource = &dataSource;
    DataSourceWidget_->UpdateDataSource(dataSource);

    RenderingWidget->UpdateImageAlgorithm(*CurrentDataSource);

    SyncSpinBoxes();
}

void ModelingWidget::showEvent(QShowEvent* event) {
    SyncSpinBoxes();

    QWidget::showEvent(event);
}

auto ModelingWidget::SyncSpinBoxes() -> void {
    PhysicalDimensionsSpinBoxes->SetRowData(
            0, DoubleCoordinateRowWidget::RowData { CurrentDataSource->GetVolumeDataPhysicalDimensions() });
    ResolutionSpinBoxes->SetRowData(
            0, IntegerCoordinateRowWidget::RowData { CurrentDataSource->GetVolumeNumberOfVoxels() });
}


DataSourceWidget::DataSourceWidget(CtStructureTree& ctStructureTree, CtDataSource& dataSource, QWidget* parent) :
        QWidget(parent),
        ImplicitButton([]() {
            auto* button = new QPushButton("Implicit Modeling");
            button->setCheckable(true);
            button->setChecked(true);
            button->setMinimumWidth(static_cast<int>(static_cast<double>(button->sizeHint().width()) * 1.2));
            return button;
        }()),
        NrrdButton([]() {
            auto* button = new QPushButton("Import .nrrd Data");
            button->setCheckable(true);
            button->setChecked(false);
            button->setMinimumWidth(static_cast<int>(static_cast<double>(button->sizeHint().width()) * 1.2));
            return button;
        }()),
        SelectSourceTypeButtonGroup([this]() {
            auto* buttonGroup = new QButtonGroup(this);
            buttonGroup->addButton(ImplicitButton);
            buttonGroup->addButton(NrrdButton);
            return buttonGroup;
        }()),
        StackedWidget([&ctStructureTree]() {
            auto* stackedWidget = new QStackedWidget();
            stackedWidget->setContentsMargins({});
            stackedWidget->addWidget(new CtStructureTreeWidget(ctStructureTree));
            stackedWidget->addWidget(new CtDataImportWidget());
            return stackedWidget;
        }()) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});
    vLayout->setSpacing(10);

    auto* titleLabel = new QLabel("Image Source");
    titleLabel->setStyleSheet(GetHeader1StyleSheet());
    vLayout->addWidget(titleLabel);

    auto* buttonsHLayout = new QHBoxLayout();
    buttonsHLayout->setContentsMargins({});
    buttonsHLayout->setSpacing(15);
    buttonsHLayout->addStretch();
    buttonsHLayout->addWidget(ImplicitButton);
    buttonsHLayout->addWidget(NrrdButton);
    buttonsHLayout->addStretch();
    vLayout->addLayout(buttonsHLayout);

    vLayout->addWidget(StackedWidget);

    connect(SelectSourceTypeButtonGroup, &QButtonGroup::buttonClicked, this, [this](QAbstractButton* button) {
        auto* pushButton = qobject_cast<QPushButton*>(button);

        using WidgetVariant = std::variant<CtStructureTreeWidget*, CtDataImportWidget*>;
        WidgetVariant newWidgetVariant = [this, pushButton]() -> WidgetVariant {
            if (pushButton == ImplicitButton)
                return dynamic_cast<CtStructureTreeWidget*>(StackedWidget->widget(0));

            if (pushButton == NrrdButton)
                return dynamic_cast<CtDataImportWidget*>(StackedWidget->widget(1));

            throw std::runtime_error("invalid button");
        }();

        auto* oldWidget = StackedWidget->currentWidget();
        auto* newWidget = std::visit([](auto* widget) { return static_cast<QWidget*>(widget); }, newWidgetVariant);
        if (newWidget == oldWidget)
            return;

        if (std::holds_alternative<CtDataImportWidget*>(newWidgetVariant)) {
            auto* importWidget = std::get<CtDataImportWidget*>(newWidgetVariant);
            auto const success = importWidget->Prepare();

            if (!success)
                return;
        }

        auto& newDataSource = std::visit([](auto* widget) -> CtDataSource& { return widget->GetCtDataSource(); },
                                         newWidgetVariant);

        App::GetInstance().SetCtDataSource(newDataSource);
    });


    bool isImplicit = dynamic_cast<ImplicitCtDataSource*>(&dataSource);
    if (!isImplicit && !dynamic_cast<NrrdCtDataSource*>(&dataSource))
        throw std::runtime_error("invalid data source type");

    if (isImplicit)
        ImplicitButton->click();
    else
        NrrdButton->click();
}

auto DataSourceWidget::UpdateDataSource(CtDataSource& dataSource) -> void {
    bool isImplicit = dynamic_cast<ImplicitCtDataSource*>(&dataSource);
    if (!isImplicit && !dynamic_cast<NrrdCtDataSource*>(&dataSource))
        throw std::runtime_error("invalid data source type");

    if (isImplicit) {
        auto& source = dynamic_cast<ImplicitCtDataSource&>(dataSource);
        auto* widget = dynamic_cast<CtStructureTreeWidget*>(StackedWidget->widget(0));
        widget->UpdateDataSource(source);
        StackedWidget->setCurrentWidget(widget);
    } else {
        auto& source = dynamic_cast<NrrdCtDataSource&>(dataSource);
        auto* widget = dynamic_cast<CtDataImportWidget*>(StackedWidget->widget(1));
        widget->UpdateDataSource(source);
        StackedWidget->setCurrentWidget(widget);
    }

    ImplicitButton->setChecked(isImplicit);
    NrrdButton->setChecked(!isImplicit);
}

CtStructureTreeWidget::CtStructureTreeWidget(CtStructureTree& ctStructureTree, QWidget* parent) :
        QWidget(parent),
        DataSource([&ctStructureTree]() {
            vtkNew<ImplicitCtDataSource> dataSource;
            dataSource->SetDataTree(&ctStructureTree);
            return dataSource;
        }()),
        AddStructureButton(new QPushButton(GenerateIcon("Plus"), " Sibling")),
        CombineWithStructureButton(new QPushButton(GenerateIcon("Plus"), " Combine")),
        RefineWithStructureButton(new QPushButton(GenerateIcon("Plus"), " Refine")),
        RemoveStructureButton(new QPushButton(GenerateIcon("Minus"), " Remove")),
        CtStructureButtons { AddStructureButton,
                             CombineWithStructureButton,
                             RefineWithStructureButton,
                             RemoveStructureButton },
        TreeView(new CtStructureView(ctStructureTree)),
        TreeModel(dynamic_cast<CtStructureTreeModel*>(TreeView->model())),
        SelectionModel(TreeView->selectionModel()),
        CtStructureCreateDialog(nullptr) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});

    auto* treeButtonBarWidget = new QWidget();
    auto* treeButtonBarHorizontalLayout = new QHBoxLayout(treeButtonBarWidget);
    treeButtonBarHorizontalLayout->setContentsMargins(0, 11, 0, 0);
    treeButtonBarHorizontalLayout->addWidget(AddStructureButton);
    treeButtonBarHorizontalLayout->addWidget(CombineWithStructureButton);
    treeButtonBarHorizontalLayout->addWidget(RefineWithStructureButton);
    treeButtonBarHorizontalLayout->addStretch();
    treeButtonBarHorizontalLayout->addWidget(RemoveStructureButton);
    vLayout->addWidget(treeButtonBarWidget);
    vLayout->addWidget(TreeView);

    DisableButtons();

    connect(AddStructureButton, &QPushButton::clicked, [&]() {
        CtStructureCreateDialog = new BasicStructureDialog(CtStructureDialog::DialogMode::CREATE, this);
        CtStructureCreateDialog->show();

        connect(CtStructureCreateDialog, &CtStructureDialog::accepted, [&]() {
            BasicStructureData const dialogData = GetWidgetData<BasicStructureWidget>(CtStructureCreateDialog);
            QModelIndex const siblingIndex = SelectionModel->currentIndex();
            QModelIndex const newIndex = TreeModel->AddBasicStructure(dialogData, siblingIndex);
            SelectionModel->clearSelection();
            SelectionModel->select(newIndex, QItemSelectionModel::SelectionFlag::Select);
        });
    });

    connect(CombineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenBasicAndCombinedStructureCreateDialog([&](BasicStructureData const& basicStructureData,
                                                      CombinedStructureData const& combinedStructureData) {
            TreeModel->CombineWithBasicStructure(basicStructureData, combinedStructureData);
        });
    });

    connect(RefineWithStructureButton, &QPushButton::clicked, [&]() {
        OpenBasicAndCombinedStructureCreateDialog([&](BasicStructureData const& basicStructureData,
                                                      CombinedStructureData const& combinedStructureData) {
            const QModelIndex index = SelectionModel->currentIndex();
            TreeModel->RefineWithBasicStructure(basicStructureData, combinedStructureData, index);
        });
    });

    connect(RemoveStructureButton, &QPushButton::clicked, [&]() {
        const QModelIndex structureIndex = SelectionModel->currentIndex();
        TreeModel->RemoveBasicStructure(structureIndex);
        SelectionModel->clearSelection();

        UpdateButtonStates({}, {});

        TreeView->expandAll();
    });

    connect(SelectionModel, &QItemSelectionModel::selectionChanged,
            this, &CtStructureTreeWidget::UpdateButtonStates);
}

CtStructureTreeWidget::~CtStructureTreeWidget() = default;

void CtStructureTreeWidget::DisableButtons() {
    for (auto const& button: CtStructureButtons)
        button->setEnabled(false);
}

void CtStructureTreeWidget::OpenBasicAndCombinedStructureCreateDialog(OnAcceptedFunction const& onAccepted) {
    auto* dialog = new BasicAndCombinedStructureCreateDialog(this);
    CtStructureCreateDialog = dialog;
    CtStructureCreateDialog->show();

    connect(CtStructureCreateDialog, &CtStructureDialog::accepted, [&, dialog, onAccepted]() {
        BasicStructureData const basicStructureData = GetWidgetData<BasicStructureWidget>(dialog);
        CombinedStructureData const combinedStructureData = GetWidgetData<CombinedStructureWidget>(dialog);

        onAccepted(basicStructureData, combinedStructureData);

        TreeView->expandAll();
    });
}

void CtStructureTreeWidget::UpdateButtonStates(QItemSelection const& selected, QItemSelection const& /*unused*/) {
    auto modelIndexList = selected.indexes();
    if (modelIndexList.size() > 1)
        throw std::runtime_error("Invalid selection");

    const QModelIndex selectedIndex = modelIndexList.size() == 1
                                      ? selected.indexes()[0]
                                      : QModelIndex();

    const bool isBasicStructure = selectedIndex.isValid()
                                  && selectedIndex.data(TreeModelRoles::IS_BASIC_STRUCTURE).toBool();

    AddStructureButton->setEnabled((isBasicStructure && selectedIndex.parent().isValid()) || !TreeModel->HasRoot());
    CombineWithStructureButton->setEnabled(selectedIndex.isValid() && !selectedIndex.parent().isValid());
    RefineWithStructureButton->setEnabled(isBasicStructure);
    RemoveStructureButton->setEnabled(isBasicStructure);
}

auto CtStructureTreeWidget::GetCtDataSource() -> CtDataSource& { return *DataSource; }

auto CtStructureTreeWidget::UpdateDataSource(ImplicitCtDataSource& dataSource) -> void {
    DataSource = &dataSource;
}


CtDataImportWidget::CtDataImportWidget(QWidget* parent) :
        QWidget(parent),
        DataSource(NrrdCtDataSource::New()),
        ImportNotice(new QLabel("...")) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});
    vLayout->addWidget(ImportNotice);
    vLayout->addStretch();
}

CtDataImportWidget::~CtDataImportWidget() = default;

auto CtDataImportWidget::GetCtDataSource() -> CtDataSource& { return *DataSource; }

auto CtDataImportWidget::Prepare() -> bool {
    auto homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homeLocations.empty() || homeLocations.at(0).isEmpty())
        throw std::runtime_error("home path must not be empty");
    QString const& homePath = homeLocations.at(0);

    QString const caption = "Import Ct Data";
    QString const fileFilter = "Ct Volumes (*.nrrd *.nhdr)";
    QString const fileName = QFileDialog::getOpenFileName(this, caption, homePath, fileFilter);
    std::filesystem::path const filePath = std::filesystem::path(fileName.toStdString());

    if (filePath.empty() || !is_regular_file(filePath))
        return false;

    DataSource->SetFilepath(filePath);

    ImportNotice->setText(filePath.string().c_str());

    return true;
}

auto CtDataImportWidget::UpdateDataSource(NrrdCtDataSource& dataSource) -> void {
    DataSource = &dataSource;
    ImportNotice->setText(DataSource->GetFilepath().string().c_str());
}
