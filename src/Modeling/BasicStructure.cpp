#include "BasicStructure.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

const QString BasicStructureBaseUi::FunctionTypeComboBoxName = "FunctionType";
const QString BasicStructureBaseUi::TissueTypeComboBoxName = "TissueType";
const QString BasicStructureBaseUi::FunctionParametersGroupName = "FunctionTypeParametersGroup";


template class CtStructureBaseData<BasicStructureBaseData<SphereStructureImpl>>;
template class CtStructureBaseData<BasicStructureBaseData<BoxStructureImpl>>;

template<TBasicStructure Impl>
auto BasicStructureBaseData<Impl>::PopulateDerivedStructure(Structure& structure) const noexcept -> void {
    structure.SetTissueType(CtStructureBase::GetTissueTypeByName(TissueName.toStdString()));

    structure.BasicStructureImpl.SetFunctionData(Data);
}

template<TBasicStructure Impl>
auto BasicStructureBaseData<Impl>::PopulateFromDerivedStructure(const Structure& structure) noexcept -> void {
    TissueName = QString::fromStdString(structure.Tissue.Name);

    structure.BasicStructureImpl.AddFunctionData(Data);
}

template<TBasicStructure Impl>
void BasicStructureBaseData<Impl>::AddSubTypeWidgets(QFormLayout* fLayout) {
    auto* functionTypeComboBox = new QComboBox();
    functionTypeComboBox->setObjectName(FunctionTypeComboBoxName);
    for (const auto &implicitFunctionAndName : CtStructureBase::GetFunctionTypeValues()) {
        functionTypeComboBox->addItem(implicitFunctionAndName.Name,
                                      QVariant::fromValue(implicitFunctionAndName.EnumValue));
    }
    functionTypeComboBox->setCurrentIndex(0);

    auto* tissueTypeComboBox = new QComboBox();
    tissueTypeComboBox->setObjectName(TissueTypeComboBoxName);
    tissueTypeComboBox->addItems(CtStructureBase::GetTissueTypeNames());

    fLayout->addRow("Tissue Type", tissueTypeComboBox);
    fLayout->addRow("Structure Type", functionTypeComboBox);

    auto* functionGroup = new QGroupBox();
    functionGroup->setObjectName(FunctionParametersGroupName);
    fLayout->addRow(functionGroup);
    UpdateFunctionParametersGroup(fLayout);

    QObject::connect(functionTypeComboBox, &QComboBox::currentIndexChanged,
                     [&, fLayout]() { UpdateFunctionParametersGroup(fLayout); });
}

template<TBasicStructure Impl>
auto BasicStructureBaseData<Impl>::PopulateStructureWidget(QWidget* widget) const -> void {
    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto* tissueTypeComboBox = widget->findChild<QComboBox*>(TissueTypeComboBoxName);

    if (int idx = functionTypeComboBox->findData(QVariant::fromValue(FunctionType));
            idx != -1)
        functionTypeComboBox->setCurrentIndex(idx);

    if (int idx = tissueTypeComboBox->findText(TissueName);
            idx != -1)
        tissueTypeComboBox->setCurrentIndex(idx);

    auto* fLayout = dynamic_cast<QFormLayout*>(functionTypeComboBox->parentWidget()->layout());
    fLayout->setRowVisible(functionTypeComboBox, false);

    UpdateFunctionParametersGroup(widget->findChild<QFormLayout*>());

    Data.PopulateWidget(widget);
}

template<TBasicStructure Impl>
auto BasicStructureBaseData<Impl>::PopulateFromStructureWidget(QWidget* widget) -> void {
    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto* tissueTypeComboBox = widget->findChild<QComboBox*>(TissueTypeComboBoxName);

    FunctionType = functionTypeComboBox->currentData().value<CtStructureBase::FunctionType>();
    TissueName = tissueTypeComboBox->currentText();

    assert(FunctionType == Impl::GetFunctionType());

    Data.PopulateFromWidget(widget);
}

template<TBasicStructure Impl>
QGroupBox* BasicStructureBaseData<Impl>::GetFunctionParametersGroup(CtStructureBase::FunctionType functionType) {
    auto* group = new QGroupBox();
    group->setObjectName(FunctionParametersGroupName);
    group->setTitle(QString::fromStdString(CtStructureBase::FunctionTypeToString(functionType)));

    auto* fLayout = new QFormLayout(group);
    fLayout->setHorizontalSpacing(15);

    switch (functionType) {
        case CtStructureBase::FunctionType::SPHERE: { SphereDataImpl::AddFunctionWidget(fLayout); break; }
        case CtStructureBase::FunctionType::BOX:    { BoxDataImpl::AddFunctionWidget(fLayout); break; }
        case CtStructureBase::FunctionType::CONE:   { qWarning("todo"); break; }
        default: throw std::runtime_error("No matching function type");
    }

    return group;
}

template<TBasicStructure Impl>
void BasicStructureBaseData<Impl>::UpdateFunctionParametersGroup(QFormLayout* fLayout) {
    auto* widget = fLayout->parentWidget();
    auto* oldFunctionParametersGroup = widget->findChild<QGroupBox*>(FunctionParametersGroupName);
    if (!oldFunctionParametersGroup)
        throw std::runtime_error("No function parameters group exists");

    auto* functionTypeComboBox = widget->findChild<QComboBox*>(FunctionTypeComboBoxName);
    auto functionType = functionTypeComboBox->currentData().value<CtStructureBase::FunctionType>();
    auto* newFunctionParametersGroup = GetFunctionParametersGroup(functionType);

    fLayout->replaceWidget(oldFunctionParametersGroup, newFunctionParametersGroup);
    delete oldFunctionParametersGroup;
}

template<TBasicStructure StructureImpl>
auto BasicStructureBase<StructureImpl>::GetViewName() const noexcept -> std::string {
    return FunctionTypeToString(StructureImpl::GetFunctionType()) + (this->Name.empty() ? "" : " (" + this->Name + ")");
}

template<TBasicStructure StructureImpl>
auto BasicStructureBase<StructureImpl>::GetFunctionType() const noexcept -> FunctionType {
    return StructureImpl::GetFunctionType();
}

auto operator<<(ostream& stream, const CtStructureBase::TissueType& type) noexcept -> std::ostream& {
    return stream << type.Name << ": ('" << type.CtNumber << "')";
}

template<TBasicStructure StructureImpl>
auto BasicStructureBase<StructureImpl>::SetTissueType(TissueType tissueType) noexcept -> void {
    Tissue = std::move(tissueType);

    Modified();
}

template<TBasicStructure StructureImpl>
auto BasicStructureBase<StructureImpl>::GetData() const noexcept -> Data {
    Data data {};
    data.PopulateFromStructure(*this);
    return data;
}

template<TBasicStructure StructureImpl>
auto BasicStructureBase<StructureImpl>::SetData(const Data& data) noexcept -> void {
    data.PopulateStructure(*this);
}

template class BasicStructureBase<SphereStructureImpl>;
template class BasicStructureBase<BoxStructureImpl>;


auto BasicStructure::CreateBasicStructure(const BasicStructureDataVariant& dataVariant) -> BasicStructureVariant {
    return std::visit(Overload{
            [&](const SphereData&) {
                SphereStructure sphere{};
                sphere.SetData(std::get<SphereData>(dataVariant));
                return BasicStructureVariant{std::move(sphere)};
            },
            [&](const BoxData&)    {
                BoxStructure box{};
                box.SetData(std::get<BoxData>(dataVariant));
                return BasicStructureVariant{std::move(box)};
            } }, dataVariant);
}


auto BasicStructureUi::GetWidgetData(QWidget* widget) -> BasicStructureDataVariant {
    auto* functionTypeComboBox = widget->findChild<QComboBox*>(BasicStructureBaseUi::FunctionTypeComboBoxName);

    auto functionType = functionTypeComboBox->currentData().value<CtStructureBase::FunctionType>();

    switch (functionType) {
        case CtStructureBase::FunctionType::SPHERE: {
            SphereData data {};
            data.PopulateFromWidget(widget);
            return data;
        }

        case CtStructureBase::FunctionType::BOX: {
            BoxData data {};
            data.PopulateFromWidget(widget);
            return data;
        }

        default: throw std::runtime_error("No matching basic structure function type");
    }
}

auto BasicStructureUi::GetWidget() -> QWidget* {
    return SphereData::GetWidget();
}
