#include "BasicImageArtifact.h"

#include "../Overload.h"
#include "../Types.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

#include <vtkImageAlgorithm.h>


auto BasicImageArtifactDetails::BasicImageArtifactDataVariant::PopulateFromArtifact(
        const BasicImageArtifact& artifact) noexcept -> void {

    *this = std::visit([&](const auto& artifact) {
        ArtifactDataT<decltype(artifact)> data {};
        data.PopulateFromArtifact(artifact);
        return BasicImageArtifactDataVariant { data };
    }, artifact.Artifact);
}

auto BasicImageArtifactDetails::BasicImageArtifactDataVariant::PopulateArtifact(
        BasicImageArtifact& artifact) const noexcept -> void {

    std::visit([dataVariant = this](auto& artifact) {
        auto& data = std::get<ArtifactDataT<decltype(artifact)>>(*dataVariant);
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
        return BasicImageArtifact::GetSubType(DataArtifactT<decltype(data)>());
    }, data);

    if (const int idx = SubTypeComboBox->findData(QVariant::fromValue(subType)); idx != -1)
        SubTypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(SubTypeComboBox, false);

    std::visit([&](auto* widget) { widget->Populate(std::get<ArtifactWidgetDataT<decltype(widget)>>(data)); },
               SubTypeWidgetVariant);
}

auto BasicImageArtifactDetails::BasicImageArtifactWidgetImpl::UpdateSubTypeWidget() noexcept -> void {
    auto subType = SubTypeComboBox->currentData().value<SubType>();

    BasicImageArtifactWidgetVariant newWidgetVariant = [subType]() {
        switch (subType) {
            case SubType::GAUSSIAN:    return BasicImageArtifactWidgetVariant { new GaussianArtifactWidget() };
            default: qWarning("Todo");
        }
        return BasicImageArtifactWidgetVariant { new GaussianArtifactWidget() };
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
        BasicImageArtifact(GetSubType(BasicImageArtifactVariant { std::visit([](auto& data) { return DataArtifactT<decltype(data)>{}; }, data.Data ) } )) {
    data.PopulateArtifact(*this);
}

BasicImageArtifact::BasicImageArtifact(SubType subType) :
        Artifact([subType]() -> BasicImageArtifactVariant {
            switch (subType) {
                case SubType::GAUSSIAN:    return GaussianArtifact();
                case SubType::SALT_PEPPER:
                case SubType::RING:
                case SubType::CUPPING:
                case SubType::WIND_MILL:
                case SubType::STAIR_STEP:
                case SubType::STREAKING:
                default: { qWarning("Todo"); return GaussianArtifact(); }
            }
        }()) {
}

auto BasicImageArtifact::GetSubType() const noexcept -> SubType {
    return GetSubType(Artifact);
}

auto BasicImageArtifact::GetSubType(const BasicImageArtifactVariant& artifact) noexcept -> SubType {
    return std::visit(Overload{
            [](const GaussianArtifact&)  { return SubType::GAUSSIAN; },
            [](auto&) { qWarning("Todo"); return SubType::GAUSSIAN; }
    }, artifact);
}

auto BasicImageArtifact::GetViewName() const noexcept -> std::string {
    std::string subTypeFullName = SubTypeToString(GetSubType());
    std::string const subTypeViewName = subTypeFullName.erase(0, subTypeFullName.find(' ') + 1);
    std::string const viewName = subTypeViewName + (Name.empty() ? "" : (" (" + Name + ")"));
    return viewName;
}

auto BasicImageArtifact::AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& {
    return std::visit([&](auto& artifact) -> vtkImageAlgorithm& {
        vtkImageAlgorithm& filter = artifact.GetFilter();
        filter.SetInputConnection(inputAlgorithm.GetOutputPort());
        return filter;
    }, Artifact);
}
