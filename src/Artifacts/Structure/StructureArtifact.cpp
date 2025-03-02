#include "StructureArtifact.h"

#include "../Types.h"
#include "../../Ui/Utils/NameLineEdit.h"
#include "../../Utils/Overload.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

#include <stdexcept>


auto StructureArtifact::GetViewName() const noexcept -> std::string {
    std::string const viewName = SubTypeToString(GetSubType()) + (Name.empty() ? "" : " (" + Name + ")");
    return viewName;
}

auto StructureArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    return std::visit([](auto& artifact) { return artifact.GetProperties(); }, Artifact);
}

auto StructureArtifact::GetSubType() const noexcept -> SubType {
    return GetSubType(Artifact);
}

auto
StructureArtifact::GetSubType(StructureArtifactVariant const& artifactVariant) -> SubType {
    return std::visit(Overload {
            [](MotionArtifact const&)   { return SubType::MOTION; },
            [](MetalArtifact const&)    { return SubType::METAL; },
            [](WindmillArtifact const&) { return SubType::WINDMILL; },
            [](auto const&) { throw std::runtime_error("invalid artifact type"); }
    }, artifactVariant);
}

StructureArtifact::StructureArtifact(StructureArtifactData const& data) :
        Artifact([&]() -> StructureArtifactVariant {
            return std::visit([](auto& data) -> StructureArtifactVariant { return ArtifactTypeT<decltype(data)>(); },
                              data.Data);
        }()) {
    data.PopulateArtifact(*this);
}


auto StructureArtifactData::PopulateFromArtifact(StructureArtifact const& artifact) noexcept -> void {
    Name = QString::fromStdString(artifact.GetName());
    Data = std::visit([&](auto& artifact) -> StructureArtifactDataVariant {
        DataTypeT<decltype(artifact)> data {};
        data.PopulateFromArtifact(artifact);
        return data;
    }, artifact.Artifact);
}

auto StructureArtifactData::PopulateArtifact(StructureArtifact& artifact) const noexcept -> void {
    artifact.SetName(Name.toStdString());
    std::visit([&](auto& artifact) { std::get<DataTypeT<decltype(artifact)>>(Data).PopulateArtifact(artifact); },
               artifact.Artifact);
}

StructureArtifactWidget::StructureArtifactWidget() :
        Layout(new QFormLayout(this)),
        NameEdit(new NameLineEdit()),
        SubTypeComboBox(new QComboBox()),
        SubTypeGroupBox(new QGroupBox()),
        WidgetVariant(new MotionArtifactWidget()) {

    Layout->setContentsMargins(0, 5, 0, 5);
    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    Layout->addRow("Name", NameEdit);

    for (auto const & [name, enumValue] : StructureArtifactDetails::GetSubTypeValues())
        SubTypeComboBox->addItem(name,
                                 QVariant::fromValue(enumValue));
    Layout->addRow("Function Type", SubTypeComboBox);

    auto* subTypeVLayout = new QVBoxLayout(SubTypeGroupBox);
    subTypeVLayout->setContentsMargins(0, 0, 0, 0);
    std::visit([subTypeVLayout](QWidget* widget) { subTypeVLayout->addWidget(widget); }, WidgetVariant);
    UpdateSubTypeWidget();
    Layout->addRow(SubTypeGroupBox);

    connect(SubTypeComboBox, &QComboBox::currentIndexChanged, [&] { UpdateSubTypeWidget(); });
}

auto StructureArtifactWidget::GetData() const noexcept -> StructureArtifactData {
    StructureArtifactData data;

    data.Name = NameEdit->GetText();
    data.Data = std::visit([](auto* widget) { return DataVariant { widget->GetData() }; },
                           WidgetVariant);

    return data;
}

auto StructureArtifactWidget::Populate(StructureArtifactData const& data) noexcept -> void {
    NameEdit->SetText(data.Name);

    StructureArtifact::SubType const subType = std::visit([&](const auto& data) {
        return StructureArtifact::GetSubType(ArtifactTypeT<decltype(data)>());
    }, data.Data);

    if (const int idx = SubTypeComboBox->findData(QVariant::fromValue(subType)); idx != -1)
        SubTypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(SubTypeComboBox, false);

    std::visit([&](auto* widget) { widget->Populate(std::get<DataTypeT<decltype(widget)>>(data.Data)); },
               WidgetVariant);
}

auto StructureArtifactWidget::UpdateSubTypeWidget() noexcept -> void {
    auto subType = SubTypeComboBox->currentData().value<StructureArtifact::SubType>();

    StructureArtifactWidgetVariant newWidgetVariant = [subType]() -> StructureArtifactWidgetVariant {
        switch (subType) {
            case StructureArtifact::SubType::MOTION:   return new MotionArtifactWidget();
            case StructureArtifact::SubType::METAL:    return new MetalArtifactWidget();
            case StructureArtifact::SubType::WINDMILL: return new WindmillArtifactWidget();
            default: throw std::runtime_error("Invalid artifact type");
        }
    }();

    std::visit([this](auto* oldSubTypeWidget) {
        SubTypeGroupBox->layout()->removeWidget(oldSubTypeWidget);
        delete oldSubTypeWidget;
    }, WidgetVariant);

    std::visit([this](auto* newSubTypeWidget) {
        SubTypeGroupBox->layout()->addWidget(newSubTypeWidget);
    }, newWidgetVariant);

    WidgetVariant = newWidgetVariant;

    SubTypeGroupBox->setTitle(QString::fromStdString(SubTypeToString(subType)));
}
