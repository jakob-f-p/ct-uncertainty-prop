#include "BasicImageArtifact.h"

#include "../../Utils/Overload.h"
#include "../../Utils/Types.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

#include <vtkImageAlgorithm.h>


auto BasicImageArtifactDetails::BasicImageArtifactDataVariant::PopulateFromArtifact(
        const BasicImageArtifact& artifact) noexcept -> void {

    *this = std::visit([&](const auto& artifact) {
        DataTypeT<decltype(artifact)> data {};
        data.PopulateFromArtifact(artifact);
        return BasicImageArtifactDataVariant { data };
    }, artifact.Artifact);
}

auto BasicImageArtifactDetails::BasicImageArtifactDataVariant::PopulateArtifact(
        BasicImageArtifact& artifact) const noexcept -> void {

    std::visit([dataVariant = this](auto& artifact) {
        auto& data = std::get<DataTypeT<decltype(artifact)>>(*dataVariant);
        data.PopulateArtifact(artifact);
    }, artifact.Artifact);
}

BasicImageArtifactDetails::BasicImageArtifactWidgetImpl::BasicImageArtifactWidgetImpl() :
        Layout(new QFormLayout(this)),
        SubTypeComboBox(new QComboBox()),
        SubTypeGroupBox(new QGroupBox()),
        SubTypeWidgetVariant(new GaussianArtifactWidget()) {

    Layout->setContentsMargins(0, 5, 0, 5);
    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    for (const auto &subTypeAndName : BasicImageArtifactDetails::GetSubTypeValues())
        SubTypeComboBox->addItem(subTypeAndName.Name,
                                 QVariant::fromValue(subTypeAndName.EnumValue));
    Layout->addRow("Function Type", SubTypeComboBox);

    auto* subTypeVLayout = new QVBoxLayout(SubTypeGroupBox);
    subTypeVLayout->setContentsMargins(0, 0, 0, 0);
    std::visit([subTypeVLayout](QWidget* widget) { subTypeVLayout->addWidget(widget); }, SubTypeWidgetVariant);
    UpdateSubTypeWidget();
    Layout->addRow(SubTypeGroupBox);

    QObject::connect(SubTypeComboBox, &QComboBox::currentIndexChanged, [&]() { UpdateSubTypeWidget(); });
}

auto BasicImageArtifactDetails::BasicImageArtifactWidgetImpl::GetData() const noexcept -> Data {
    return std::visit([](auto* widget) { return BasicImageArtifactDataVariant { widget->GetData() }; },
                      SubTypeWidgetVariant);
}

auto BasicImageArtifactDetails::BasicImageArtifactWidgetImpl::Populate(const Data& data) noexcept -> void {
    SubType subType = std::visit([&](const auto& data) {
        return BasicImageArtifact::GetSubType(ArtifactTypeT<decltype(data)>());
    }, data);

    if (const int idx = SubTypeComboBox->findData(QVariant::fromValue(subType)); idx != -1)
        SubTypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(SubTypeComboBox, false);

    std::visit([&](auto* widget) { widget->Populate(std::get<DataTypeT<decltype(widget)>>(data)); },
               SubTypeWidgetVariant);
}

auto BasicImageArtifactDetails::BasicImageArtifactWidgetImpl::UpdateSubTypeWidget() noexcept -> void {
    auto subType = SubTypeComboBox->currentData().value<SubType>();

    BasicImageArtifactWidgetVariant newWidgetVariant = [subType]() -> BasicImageArtifactWidgetVariant {
        switch (subType) {
            case SubType::GAUSSIAN:    return new GaussianArtifactWidget();
            case SubType::SALT_PEPPER: return new SaltPepperArtifactWidget();
            case SubType::RING:        return new RingArtifactWidget();
            case SubType::CUPPING:     return new CuppingArtifactWidget();
            case SubType::WIND_MILL:   return new WindMillArtifactWidget();
            case SubType::STAIR_STEP:  return new StairStepArtifactWidget();
            default: qWarning("Todo");
        }
        return new GaussianArtifactWidget();
    }();

    std::visit([&, newWidgetVariant](auto* oldSubTypeWidget) {
        std::visit([&, oldSubTypeWidget](auto* newSubTypeWidget) {
            SubTypeGroupBox->layout()->replaceWidget(oldSubTypeWidget, newSubTypeWidget);
            delete oldSubTypeWidget;

            SubTypeWidgetVariant = newWidgetVariant;
        }, newWidgetVariant);
    }, SubTypeWidgetVariant);

    SubTypeGroupBox->setTitle(QString::fromStdString(SubTypeToString(subType)));
}



BasicImageArtifact::BasicImageArtifact(const BasicImageArtifactData& data) :
        BasicImageArtifact(std::visit([](auto& data) {
                return BasicImageArtifactVariant { ArtifactTypeT<decltype(data)>{} };
            }, data.Data)) {
    data.PopulateArtifact(*this);
}

auto BasicImageArtifact::GetSubType() const noexcept -> SubType {
    return GetSubType(Artifact);
}

auto BasicImageArtifact::GetSubType(const BasicImageArtifactVariant& artifact) noexcept -> SubType {
    return std::visit(Overload{
            [](const GaussianArtifact&)   { return SubType::GAUSSIAN; },
            [](const SaltPepperArtifact&) { return SubType::SALT_PEPPER; },
            [](const RingArtifact&)       { return SubType::RING; },
            [](const CuppingArtifact&)    { return SubType::CUPPING; },
            [](const WindMillArtifact&)   { return SubType::WIND_MILL; },
            [](const StairStepArtifact&)  { return SubType::STAIR_STEP; },
            [](auto&) { qWarning("Todo"); return SubType::GAUSSIAN; }
    }, artifact);
}

auto BasicImageArtifact::GetViewName() const noexcept -> std::string {
    return SubTypeToString(GetSubType()) + (Name.empty() ? "" : (" (" + Name + ")"));
}

auto BasicImageArtifact::AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& {
    return std::visit([&](auto& artifact) -> vtkImageAlgorithm& {
        vtkImageAlgorithm& filter = artifact.GetFilter();
        filter.SetInputConnection(inputAlgorithm.GetOutputPort());
        return filter;
    }, Artifact);
}
