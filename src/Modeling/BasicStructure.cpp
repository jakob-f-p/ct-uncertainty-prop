#include "BasicStructure.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

#define BASIC_SHAPE_DATA(Shape) \
    typename std::decay_t<decltype(Shape)>::Data

auto BasicStructureDetails::BasicStructureDataImpl::PopulateFromStructure(const Structure& structure) noexcept
        -> void {
    FunctionType = structure.GetFunctionType();
    TissueName = QString::fromStdString(structure.Tissue.Name);
    Data = std::visit([&](const auto& shape) { return BasicStructureSubTypeDataVariant{ BASIC_SHAPE_DATA(shape){} }; },
                      structure.Shape);

    std::visit([&](const auto& shape) { shape.AddFunctionData(std::get<BASIC_SHAPE_DATA(shape)>(Data)); },
               structure.Shape);
}

auto BasicStructureDetails::BasicStructureDataImpl::PopulateStructure(Structure& structure) const noexcept
        -> void {
    structure.SetTissueType(BasicStructureDetails::GetTissueTypeByName(TissueName.toStdString()));

    std::visit([&](auto& shape) -> void { shape.SetFunctionData(std::get<BASIC_SHAPE_DATA(shape)>(Data)); },
               structure.Shape);
}

BasicStructure::BasicStructure(FunctionType functionType) :
        Shape([functionType]() -> ShapeVariant { switch (functionType) {
            case FunctionType::SPHERE: return Sphere();
            case FunctionType::BOX:    return Box();
            case FunctionType::CONE: { qWarning("Todo");
                                       return Sphere(); }
            return {};
        } }()) {
}

BasicStructure::BasicStructure(const BasicStructureData& data) : BasicStructure(data.Data.FunctionType) {
    SetData(data);
}

auto BasicStructure::GetViewName() const noexcept -> std::string {
    return FunctionTypeToString(GetFunctionType()) + (this->Name.empty() ? "" : " (" + this->Name + ")");
}

auto BasicStructure::GetFunctionType() const noexcept -> FunctionType {
    return std::visit(Overload{
        [](const Sphere&) { return FunctionType::SPHERE; },
        [](const Box&)    { return FunctionType::BOX; },
        [](const auto&)       { qWarning("Invalid function type"); return FunctionType::SPHERE; },
    }, Shape);
}

auto BasicStructure::SetTissueType(TissueType tissueType) noexcept -> void {
    Tissue = std::move(tissueType);

    Modified();
}


auto BasicStructure::GetData() const noexcept -> Data {
    Data data {};
    data.PopulateFromStructure(*this);
    return data;
}

auto BasicStructure::SetData(const Data& data) noexcept -> void {
    data.PopulateStructure(*this);
}

std::atomic<StructureId> BasicStructure::GlobalBasicStructureId = 0;

BasicStructureDetails::BasicStructureWidgetImpl::BasicStructureWidgetImpl() :
        Layout(new QFormLayout(this)),
        TissueTypeComboBox  (new QComboBox()),
        FunctionTypeComboBox(new QComboBox()),
        SubTypeGroupBox(new QGroupBox()),
        SubTypeWidgetVariant(new SphereWidget()) {

    Layout->setContentsMargins(0, 5, 0, 5);
    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    TissueTypeComboBox->addItems(BasicStructureDetails::GetTissueTypeNames());
    Layout->addRow("Tissue Type", TissueTypeComboBox);

    for (const auto &implicitFunctionAndName : BasicStructureDetails::GetFunctionTypeValues())
        FunctionTypeComboBox->addItem(implicitFunctionAndName.Name,
                                      QVariant::fromValue(implicitFunctionAndName.EnumValue));
    Layout->addRow("Structure Type", FunctionTypeComboBox);

    auto* subTypeVLayout = new QVBoxLayout(SubTypeGroupBox);
    subTypeVLayout->setContentsMargins(0, 0, 0, 0);
    std::visit([subTypeVLayout](QWidget* widget) { subTypeVLayout->addWidget(widget); }, SubTypeWidgetVariant);
    UpdateFunctionParametersGroup();
    Layout->addRow(SubTypeGroupBox);

    QObject::connect(FunctionTypeComboBox, &QComboBox::currentIndexChanged, [&]() { UpdateFunctionParametersGroup(); });
}

auto BasicStructureDetails::BasicStructureWidgetImpl::AddData(Data& data) noexcept -> void {
    data.TissueName = TissueTypeComboBox->currentText();
    data.FunctionType = FunctionTypeComboBox->currentData().value<FunctionType>();
    data.Data = std::visit([](auto* widget) { return BasicStructureSubTypeDataVariant{ widget->GetData() }; },
                           SubTypeWidgetVariant);
}

#define BASIC_DATA_WIDGET(Data) \
    typename std::decay_t<decltype(Data)>::Widget

auto BasicStructureDetails::BasicStructureWidgetImpl::Populate(const Data& data) noexcept -> void {
    TissueTypeComboBox->setCurrentText(data.TissueName);

    if (const int idx = FunctionTypeComboBox->findData(QVariant::fromValue(data.FunctionType)); idx != -1)
        FunctionTypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(FunctionTypeComboBox, false);

    std::visit([widgetVariant = SubTypeWidgetVariant](const auto& subTypeData) -> void {
        auto* widget = std::get<BASIC_DATA_WIDGET(subTypeData)*>(widgetVariant);
        widget->Populate(subTypeData);
    }, data.Data);
}

auto BasicStructureDetails::BasicStructureWidgetImpl::UpdateFunctionParametersGroup() -> void {
    auto functionType = FunctionTypeComboBox->currentData().value<FunctionType>();

    BasicStructureSubTypeWidgetVariant newSubTypeWidgetVariant = [functionType]() {
        switch (functionType) {
            case FunctionType::SPHERE: { return BasicStructureSubTypeWidgetVariant{ new SphereWidget() }; }
            case FunctionType::BOX:    { return BasicStructureSubTypeWidgetVariant{ new BoxWidget() }; }
            case FunctionType::CONE:   { qWarning("todo"); return BasicStructureSubTypeWidgetVariant{}; }
        }
        return BasicStructureSubTypeWidgetVariant{};
    }();

    std::visit([&, newSubTypeWidgetVariant](auto* oldSubTypeWidget) {
        std::visit([&, oldSubTypeWidget](auto* newSubTypeWidget) {
            SubTypeGroupBox->layout()->replaceWidget(oldSubTypeWidget, newSubTypeWidget);
            delete oldSubTypeWidget;

            SubTypeWidgetVariant = newSubTypeWidgetVariant;
        }, newSubTypeWidgetVariant);
    }, SubTypeWidgetVariant);

    SubTypeGroupBox->setTitle(QString::fromStdString(BasicStructureDetails::FunctionTypeToString(functionType)));
}

#undef BASIC_SHAPE_DATA
#undef BASIC_DATA_WIDGET
