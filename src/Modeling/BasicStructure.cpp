#include "BasicStructure.h"

#include "../Utils/Overload.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

auto BasicStructureDetails::BasicStructureDataImpl::PopulateFromStructure(const Structure& structure) noexcept
        -> void {
    FunctionType = structure.GetFunctionType();
    TissueName = QString::fromStdString(structure.Tissue.Name);
    Data = std::visit([&](const auto& shape) { return ShapeDataVariant { DataTypeT<decltype(shape)>{} }; },
                      structure.Shape);

    std::visit([&](const auto& shape) { shape.AddFunctionData(std::get<DataTypeT<decltype(shape)>>(Data)); },
               structure.Shape);
}

auto BasicStructureDetails::BasicStructureDataImpl::PopulateStructure(Structure& structure) const noexcept
        -> void {
    structure.SetTissueType(BasicStructureDetails::GetTissueTypeByName(TissueName.toStdString()));

    std::visit([&](auto& shape) -> void { shape.SetFunctionData(std::get<DataTypeT<decltype(shape)>>(Data)); },
               structure.Shape);
}

BasicStructure::BasicStructure(FunctionType functionType) :
        Shape([=]() -> ShapeVariant {
            switch (functionType) {
                case FunctionType::SPHERE:   return Sphere();
                case FunctionType::BOX:      return Box();
                case FunctionType::CONE:     return Cone();
                case FunctionType::CYLINDER: return Cylinder();
                default: return {};
            }
        }()) {
}

BasicStructure::BasicStructure(const BasicStructureData& data) : BasicStructure(data.Data.FunctionType) {
    data.PopulateStructure(*this);
}

auto BasicStructure::GetViewName() const noexcept -> std::string {
    return FunctionTypeToString(GetFunctionType()) + (GetName().empty() ? "" : " (" + GetName() + ")");
}

auto BasicStructure::GetFunctionType() const noexcept -> FunctionType {
    return std::visit(Overload {
        [](Sphere const&)   { return FunctionType::SPHERE; },
        [](Box const&)      { return FunctionType::BOX; },
        [](Cone const&)     { return FunctionType::CONE; },
        [](Cylinder const&) { return FunctionType::CYLINDER; },
        [](auto const&) { qWarning("Invalid function type"); return FunctionType::SPHERE; },
    }, Shape);
}

auto BasicStructure::SetTissueType(TissueType tissueType) noexcept -> void {
    Tissue = std::move(tissueType);

    Modified();
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

auto BasicStructureDetails::BasicStructureWidgetImpl::GetData() noexcept -> Data {
    Data data {};

    data.TissueName = TissueTypeComboBox->currentText();
    data.FunctionType = FunctionTypeComboBox->currentData().value<FunctionType>();
    data.Data = std::visit([](auto* widget) { return ShapeDataVariant{ widget->GetData() }; },
                           SubTypeWidgetVariant);

    return data;
}

auto BasicStructureDetails::BasicStructureWidgetImpl::Populate(const Data& data) noexcept -> void {
    TissueTypeComboBox->setCurrentText(data.TissueName);

    if (const int idx = FunctionTypeComboBox->findData(QVariant::fromValue(data.FunctionType)); idx != -1)
        FunctionTypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(FunctionTypeComboBox, false);

    std::visit([widgetVariant = SubTypeWidgetVariant](const auto& subTypeData) -> void {
        auto* widget = std::get<WidgetTypeT<decltype(subTypeData)>*>(widgetVariant);
        widget->Populate(subTypeData);
    }, data.Data);
}

auto BasicStructureDetails::BasicStructureWidgetImpl::UpdateFunctionParametersGroup() -> void {
    auto functionType = FunctionTypeComboBox->currentData().value<FunctionType>();

    ShapeWidgetVariant newSubTypeWidgetVariant = [functionType]() {
        switch (functionType) {
            case FunctionType::SPHERE:   return ShapeWidgetVariant { new SphereWidget() };
            case FunctionType::BOX:      return ShapeWidgetVariant { new BoxWidget() };
            case FunctionType::CONE:     return ShapeWidgetVariant { new ConeWidget() };
            case FunctionType::CYLINDER: return ShapeWidgetVariant { new CylinderWidget() };
            default: throw std::runtime_error("invalid function type");
        }
    }();

    std::visit([this](auto* oldSubTypeWidget) {
        SubTypeGroupBox->layout()->removeWidget(oldSubTypeWidget);
        delete oldSubTypeWidget;
    }, SubTypeWidgetVariant);

    std::visit([this](auto* newSubTypeWidget) {
        SubTypeGroupBox->layout()->addWidget(newSubTypeWidget);
    }, newSubTypeWidgetVariant);

    SubTypeWidgetVariant = newSubTypeWidgetVariant;

    SubTypeGroupBox->setTitle(QString::fromStdString(BasicStructureDetails::FunctionTypeToString(functionType)));
}
