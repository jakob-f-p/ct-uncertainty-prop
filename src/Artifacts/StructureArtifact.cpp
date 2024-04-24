#include "StructureArtifact.h"

#include "../NameLineEdit.h"
#include "../Overload.h"
#include "../Types.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

auto StructureArtifact::GetViewName() const noexcept -> std::string {
    std::string subTypeFullName = SubTypeToString(GetSubType());
    std::string const subTypeViewName = subTypeFullName.erase(0, subTypeFullName.find(' ') + 1);
    std::string const viewName = subTypeViewName + (Name.empty() ? "" : (" (" + Name + ")"));
    return viewName;
}

auto StructureArtifact::EvaluateAtPosition(const FloatPoint& point) -> float {
    return 0;
}

auto StructureArtifact::GetSubType() const noexcept -> StructureArtifact::SubType {
    return GetSubType(Artifact);
}

auto
StructureArtifact::GetSubType(const StructureArtifactVariant& artifactVariant) noexcept -> StructureArtifact::SubType {
    return std::visit(Overload {
            [](MotionArtifact const&)  { return SubType::MOTION; },
            [](auto const&) { qWarning("Todo"); return SubType::MOTION; }
    }, artifactVariant);
}

StructureArtifact::StructureArtifact(StructureArtifact::SubType subType) :
        Artifact([subType]() -> StructureArtifactVariant {
            switch (subType) {
                case SubType::MOTION:    return MotionArtifact();
                case SubType::STREAKING:
                case SubType::METALLIC:
                default: { qWarning("Todo"); return MotionArtifact(); }
            }
        }()){
}

StructureArtifact::StructureArtifact(const StructureArtifactData& data) :
        Artifact([&]() -> StructureArtifactVariant {
            return std::visit([](auto& data) { return DataArtifactT<decltype(data)>(); }, data.Data);
        }()) {
    data.PopulateArtifact(*this);
}


auto StructureArtifactData::PopulateFromArtifact(const StructureArtifact& artifact) noexcept -> void {
    Name = QString::fromStdString(artifact.GetName());
    Data = std::visit([&](auto& artifact) {
        ArtifactDataT<decltype(artifact)> data {};
        data.PopulateFromArtifact(artifact);
        return data;
    }, artifact.Artifact);
}

auto StructureArtifactData::PopulateArtifact(StructureArtifact& artifact) const noexcept -> void {
    artifact.SetName(Name.toStdString());
    std::visit([&](auto& artifact) { std::get<ArtifactDataT<decltype(artifact)>>(Data).PopulateArtifact(artifact); },
               artifact.Artifact);

    artifact.MTime.Modified();
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

    for (const auto &subTypeAndName : StructureArtifactDetails::GetSubTypeValues())
        SubTypeComboBox->addItem(subTypeAndName.Name,
                                 QVariant::fromValue(subTypeAndName.EnumValue));
    Layout->addRow("Function Type", SubTypeComboBox);

    auto* subTypeVLayout = new QVBoxLayout(SubTypeGroupBox);
    subTypeVLayout->setContentsMargins(0, 0, 0, 0);
    std::visit([subTypeVLayout](QWidget* widget) { subTypeVLayout->addWidget(widget); }, WidgetVariant);
    UpdateSubTypeWidget();
    Layout->addRow(SubTypeGroupBox);

    QObject::connect(SubTypeComboBox, &QComboBox::currentIndexChanged, [&]() { UpdateSubTypeWidget(); });
}

auto StructureArtifactWidget::GetData() const noexcept -> StructureArtifactData {
    StructureArtifactData data;

    data.Name = NameEdit->GetData();
    data.Data = std::visit([](auto* widget) { return StructureArtifactDataVariant { widget->GetData() }; },
                           WidgetVariant);

    return data;
}

auto StructureArtifactWidget::Populate(const StructureArtifactData& data) noexcept -> void {
    NameEdit->SetData(data.Name);

    StructureArtifact::SubType const subType = std::visit([&](const auto& data) {
        return StructureArtifact::GetSubType(DataArtifactT<decltype(data)>());
    }, data.Data);

    if (const int idx = SubTypeComboBox->findData(QVariant::fromValue(subType)); idx != -1)
        SubTypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(SubTypeComboBox, false);

    std::visit([&](auto* widget) { widget->Populate(std::get<ArtifactWidgetDataT<decltype(widget)>>(data.Data)); },
               WidgetVariant);
}

auto StructureArtifactWidget::FindWidget(QWidget* widget) -> StructureArtifactWidget& {
    if (!widget)
        throw std::runtime_error("Given widget must not be nullptr");

    auto* structureArtifactWidget = widget->findChild<StructureArtifactWidget*>();

    if (!structureArtifactWidget)
        throw std::runtime_error("No image artifact widget contained in given widget");

    return *structureArtifactWidget;
}

auto StructureArtifactWidget::UpdateSubTypeWidget() noexcept -> void {
    auto subType = SubTypeComboBox->currentData().value<StructureArtifact::SubType>();

    StructureArtifactWidgetVariant newWidgetVariant = [subType]() {
        switch (subType) {
            case StructureArtifact::SubType::MOTION: return StructureArtifactWidgetVariant { new MotionArtifactWidget() };
            default: qWarning("Todo");
        }
        return StructureArtifactWidgetVariant {};
    }();

    std::visit([&, newWidgetVariant](auto* oldSubTypeWidget) {
        std::visit([&, oldSubTypeWidget](auto* newSubTypeWidget) {
            SubTypeGroupBox->layout()->replaceWidget(oldSubTypeWidget, newSubTypeWidget);
            delete oldSubTypeWidget;

            WidgetVariant = newWidgetVariant;
        }, newWidgetVariant);
    }, WidgetVariant);

    SubTypeGroupBox->setTitle(QString::fromStdString(StructureArtifactDetails::SubTypeToString(subType)));
}
