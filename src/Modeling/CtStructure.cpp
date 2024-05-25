#include "CtStructure.h"

#include "BasicStructure.h"
#include "CombinedStructure.h"
#include "../Ui/Utils/NameLineEdit.h"

#include <QFormLayout>
#include <QGroupBox>

template class CtStructureBaseData<BasicStructureDetails::BasicStructureDataImpl>;
template class CtStructureBaseData<CombinedStructureDetails::CombinedStructureDataImpl>;

template<TStructureData StructureData>
auto CtStructureBaseData<StructureData>::PopulateStructure(Structure& structure) const -> void {
    structure.SetName(Name.toStdString());
    structure.SetTransformData(Transform);

    Data.PopulateStructure(structure);
}

template<TStructureData StructureData>
auto CtStructureBaseData<StructureData>::PopulateFromStructure(const Structure& structure) -> void {
    Name = QString::fromStdString(structure.Name);
    ViewName = QString::fromStdString(structure.GetViewName());
    Transform = structure.GetTransformData();

    Data.PopulateFromStructure(structure);
}

template class CtStructureBaseWidget<BasicStructureDetails::BasicStructureWidgetImpl, BasicStructureData>;
template class CtStructureBaseWidget<CombinedStructureDetails::CombinedStructureWidgetImpl, CombinedStructureData>;

template<TStructureWidget StructureWidget, typename Data>
CtStructureBaseWidget<StructureWidget, Data>::CtStructureBaseWidget() :
        Layout(new QFormLayout(this)),
        NameEdit(new NameLineEdit()),
        SubWidget(new StructureWidget()),
        TransformWidget(new SimpleTransformWidget) {

    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    Layout->addRow("Name", NameEdit);

    Layout->addRow(SubWidget);

    auto* transformGroup = new QGroupBox("Transform");
    auto* transformLayout = new QVBoxLayout(transformGroup);
    transformLayout->addWidget(TransformWidget);
    Layout->addRow(transformGroup);
}

template<TStructureWidget StructureWidget, typename Data>
auto CtStructureBaseWidget<StructureWidget, Data>::GetData() const noexcept -> Data {
    Data data {};

    data.Name = NameEdit->GetText();
    data.Transform = TransformWidget->GetData();

    data.Data = SubWidget->GetData();

    return data;
}

template<TStructureWidget StructureWidget, typename Data>
auto CtStructureBaseWidget<StructureWidget, Data>::Populate(const Data& data) noexcept -> void {
    NameEdit->SetText(data.Name);
    TransformWidget->SetData(data.Transform);

    SubWidget->Populate(data.Data);
}