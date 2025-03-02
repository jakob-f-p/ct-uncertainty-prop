#include "ImageArtifact.h"

#include "../../Utils/Overload.h"
#include "../../PipelineGroups/ObjectProperty.h"

#include <QComboBox>
#include <QFormLayout>

ImageArtifact::ImageArtifact(ImageArtifactData const& data) noexcept :
        Artifact([&data] {
            return std::visit(Overload {
                [](BasicImageArtifactData const& basicData) -> ImageArtifactVariant {
                    return BasicImageArtifact(basicData);
                },
                [](CompositeImageArtifactData const& compositeData) -> ImageArtifactVariant {
                    return CompositeImageArtifact(compositeData);
                }
            }, data.Data);
        }()) {}

ImageArtifact::ImageArtifact(BasicImageArtifact&& basicImageArtifact) :
        Artifact(std::move(basicImageArtifact)) {
}

ImageArtifact::ImageArtifact(CompositeImageArtifact&& compositeImageArtifact) :
        Artifact(std::move(compositeImageArtifact)) {
}

auto ImageArtifact::GetViewName() const noexcept -> std::string {
    return std::visit([](auto const& artifact) { return artifact.GetViewName(); }, Artifact);
}

auto ImageArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    return std::visit([](auto& artifact) { return artifact.GetProperties(); }, Artifact);
}

auto ImageArtifact::ContainsImageArtifact(const ImageArtifact& imageArtifact) -> bool {
    return std::visit(Overload {
            [] (BasicImageArtifact&)               { return false; },
            [&](CompositeImageArtifact const& composite) { return composite.ContainsImageArtifact(imageArtifact); },
    }, Artifact);
}

auto ImageArtifact::GetParent() const -> ImageArtifact* {
    return std::visit([](auto& artifact) { return artifact.GetParent(); }, Artifact);
}

auto ImageArtifact::SetParent(ImageArtifact* parent) -> void {
    std::visit([=](auto& artifact) { artifact.SetParent(parent); }, Artifact);
}

auto ImageArtifact::ToComposite() -> CompositeImageArtifact& {
    return std::get<CompositeImageArtifact>(Artifact);
}

auto ImageArtifact::ToCompositeConst() const -> CompositeImageArtifact const& {
    return std::get<CompositeImageArtifact>(Artifact);
}

auto ImageArtifact::IsComposite() const noexcept -> bool {
    return std::visit(Overload {
            [] (BasicImageArtifact const&)     { return false; },
            [&](CompositeImageArtifact const&) { return true; }
    }, Artifact);
}

auto ImageArtifact::NumberOfChildren() const noexcept -> uint16_t {
    return std::visit(Overload {
            [] (BasicImageArtifact const&) -> uint8_t { return 0; },
            [&](CompositeImageArtifact const& composite) { return composite.NumberOfChildren(); }
    }, Artifact);
}

auto ImageArtifact::Get(uint16_t targetIdx, uint16_t& currentIdx) -> ImageArtifact* {
    if (currentIdx == targetIdx)
        return this;

    currentIdx++;

    return std::visit(Overload {
            [&](BasicImageArtifact&) -> ImageArtifact*  { return nullptr; },
            [&](CompositeImageArtifact const& composite) { return composite.Get(targetIdx, currentIdx); }
    }, Artifact);
}

auto ImageArtifact::IndexOf(ImageArtifact const& imageArtifact, uint16_t& currentIdx) const -> int32_t {
    if (this == &imageArtifact)
        return currentIdx;

    currentIdx++;

    return std::visit(Overload {
            [&](BasicImageArtifact const&) -> int32_t { return -1; },
            [&](CompositeImageArtifact const& composite) { return composite.IndexOf(imageArtifact, currentIdx); }
    }, Artifact);
}

auto ImageArtifact::AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& {
    return std::visit([&](auto& artifact) -> vtkImageAlgorithm& { return artifact.AppendImageFilters(inputAlgorithm); },
                      Artifact);
}

ImageArtifactData::ImageArtifactData(const ImageArtifact& artifact) :
        Data([&] {
            return std::visit(Overload {
                    [&](BasicImageArtifact const&) -> ImageArtifactDataVariant {
                        return BasicImageArtifactData{};
                    },
                    [&](CompositeImageArtifact const&) -> ImageArtifactDataVariant {
                        return CompositeImageArtifactData{};
                    }
            }, artifact.Artifact);
        }()) {
    PopulateFromArtifact(artifact);
}

ImageArtifactData::ImageArtifactData(BasicImageArtifactData&& data) : Data(std::move(data)) {}

ImageArtifactData::ImageArtifactData(CompositeImageArtifactData&& data) : Data(std::move(data)) {}

auto ImageArtifactData::PopulateFromArtifact(const ImageArtifact& imageArtifact) noexcept -> void {
    std::visit(Overload {
            [&](BasicImageArtifactData& basic) {
                basic.PopulateFromArtifact(std::get<BasicImageArtifact>(imageArtifact.Artifact));
            },
            [&](CompositeImageArtifactData& composite) {
                composite.PopulateFromArtifact(std::get<CompositeImageArtifact>(imageArtifact.Artifact));
            }
    }, Data);
}

auto ImageArtifactData::PopulateArtifact(ImageArtifact& imageArtifact) const noexcept -> void {
    std::visit(Overload {
            [&](BasicImageArtifactData const& basic) {
                basic.PopulateArtifact(std::get<BasicImageArtifact>(imageArtifact.Artifact)); },
            [&](CompositeImageArtifactData const& composite) {
                composite.PopulateArtifact(std::get<CompositeImageArtifact>(imageArtifact.Artifact)); }
    }, Data);
}


ImageArtifactWidget::ImageArtifactWidget() :
        Layout(new QFormLayout(this)),
        TypeComboBox(new QComboBox()),
        TypeWidget(new BasicImageArtifactWidget()) {

    Layout->setContentsMargins(0, 0, 0, 0);
    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    TypeComboBox->addItem(QString::fromStdString(TypeToString(Type::BASIC)), QVariant::fromValue(Type::BASIC));
    TypeComboBox->addItem(QString::fromStdString(TypeToString(Type::COMPOSITE)), QVariant::fromValue(Type::COMPOSITE));
    Layout->addRow("Type", TypeComboBox);

    std::visit([&](auto* widget) { Layout->addRow(widget); }, TypeWidget);

    connect(TypeComboBox, &QComboBox::currentIndexChanged, [&] { UpdateTypeWidget(); });
}

auto ImageArtifactWidget::GetData() const noexcept -> ImageArtifactData {
    return std::visit([](auto* widget) { return ImageArtifactData { widget->GetData() }; }, TypeWidget);
}

auto ImageArtifactWidget::Populate(const ImageArtifactData& data) noexcept -> void {
    Type const type = std::visit(Overload {
        [&](BasicImageArtifactData const&) { return Type::BASIC; },
        [&](CompositeImageArtifactData const&) { return Type::COMPOSITE; }
    }, data.Data);

    if (const int idx = TypeComboBox->findData(QVariant::fromValue(type)); idx != -1)
        TypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(TypeComboBox, false);

    std::visit(Overload {
            [&](BasicImageArtifactWidget* widget) { widget->Populate(std::get<BasicImageArtifactData>(data.Data)); },
            [&](CompositeImageArtifactWidget* widget) { widget->Populate(std::get<CompositeImageArtifactData>(data.Data)); }
    }, TypeWidget);
}

auto ImageArtifactWidget::UpdateTypeWidget() -> void {
    auto type = TypeComboBox->currentData().value<Type>();

    TypeWidgetVariant newWidgetVariant = [type] {
        switch (type) {
            case Type::BASIC:     return TypeWidgetVariant { new BasicImageArtifactWidget() };
            case Type::COMPOSITE: return TypeWidgetVariant { new CompositeImageArtifactWidget() };
            default: throw std::runtime_error("invalid type");
        }
    }();

    std::visit([this](auto* oldSubTypeWidget) { Layout->removeRow(oldSubTypeWidget); }, TypeWidget);

    std::visit([this](auto* newSubTypeWidget) { Layout->addRow(newSubTypeWidget); }, newWidgetVariant);

    TypeWidget = newWidgetVariant;
}

auto ImageArtifactWidget::TypeToString(Type type) -> std::string {
    switch (type) {
        case Type::BASIC:     return "Artifact";
        case Type::COMPOSITE: return "Composition";
        default: throw std::runtime_error("invalid type");
    }
}

auto ImageArtifactWidget::FindWidget(QWidget const* widget) -> ImageArtifactWidget& {
    if (!widget)
        throw std::runtime_error("Given widget must not be nullptr");

    auto* imageArtifactWidget = widget->findChild<ImageArtifactWidget*>();

    if (!imageArtifactWidget)
        throw std::runtime_error("No image artifact widget contained in given widget");

    return *imageArtifactWidget;
}
