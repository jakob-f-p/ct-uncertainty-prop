#include "ImageArtifact.h"

#include "../Overload.h"

#include <QComboBox>
#include <QFormLayout>

ImageArtifact::ImageArtifact(ImageArtifactData const& data) {
        Artifact = std::visit(Overload {
                [](BasicImageArtifactData const& basicData) -> ImageArtifactVariant { return BasicImageArtifact(basicData); },
                [](CompositeImageArtifactData const& compositeData) -> ImageArtifactVariant { return CompositeImageArtifact(compositeData); }
        }, data.Data);
}

ImageArtifact::ImageArtifact(BasicImageArtifact&& basicImageArtifact) :
        Artifact(std::move(basicImageArtifact)) {
}

ImageArtifact::ImageArtifact(CompositeImageArtifact&& compositeImageArtifact) :
        Artifact(std::move(compositeImageArtifact)) {
}

auto ImageArtifact::GetViewName() const noexcept -> std::string {
    return std::visit([](auto const& artifact) { return artifact.GetViewName(); }, Artifact);
}

auto ImageArtifact::ContainsImageArtifact(const ImageArtifact& imageArtifact) -> bool {
    return std::visit(Overload {
            [] (BasicImageArtifact&)               { return false; },
            [&](CompositeImageArtifact& composite) { return composite.ContainsImageArtifact(imageArtifact); },
    }, Artifact);
}

auto ImageArtifact::GetParent() const -> ImageArtifact* {
    return std::visit([](auto& artifact) { return artifact.GetParent(); }, Artifact);
}

auto ImageArtifact::SetParent(ImageArtifact* parent) -> void {
    return std::visit([=](auto& artifact) { return artifact.SetParent(parent); }, Artifact);
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
            [&](CompositeImageArtifact& composite) { return composite.Get(targetIdx, currentIdx); }
    }, Artifact);
}

auto ImageArtifact::IndexOf(const ImageArtifact& imageArtifact, uint16_t& currentIdx) const -> int32_t {
    if (this == &imageArtifact)
        return currentIdx;

    currentIdx++;

    return std::visit(Overload {
            [&](BasicImageArtifact const&) -> int32_t { return -1; },
            [&](CompositeImageArtifact const& composite) { return composite.IndexOf(imageArtifact, currentIdx); }
    }, Artifact);
}

auto ImageArtifact::AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& {
    return std::visit([&](auto& artifact) -> vtkImageAlgorithm& { return artifact.AppendImageFilters(inputAlgorithm); }, Artifact);
}

ImageArtifactData::ImageArtifactData(const ImageArtifact& artifact) :
        Data([&]() {
            return std::visit(Overload {
                    [&](const BasicImageArtifact& basic) -> ImageArtifactDataVariant { return BasicImageArtifactData{}; },
                    [&](const CompositeImageArtifact& composite) -> ImageArtifactDataVariant { return CompositeImageArtifactData{}; }
            }, artifact.Artifact);
        }()) {
    PopulateFromArtifact(artifact);
}

ImageArtifactData::ImageArtifactData(BasicImageArtifactData&& data) :
        Data(data) { }

ImageArtifactData::ImageArtifactData(CompositeImageArtifactData&& data) :
        Data(data) { }

auto ImageArtifactData::PopulateFromArtifact(const ImageArtifact& imageArtifact) noexcept -> void {
    std::visit(Overload {
            [&](BasicImageArtifactData& basic) { basic.PopulateFromArtifact(std::get<BasicImageArtifact>(imageArtifact.Artifact)); },
            [&](CompositeImageArtifactData& composite) { composite.PopulateFromArtifact(std::get<CompositeImageArtifact>(imageArtifact.Artifact)); }
    }, Data);
}

auto ImageArtifactData::PopulateArtifact(ImageArtifact& imageArtifact) const noexcept -> void {
    std::visit(Overload {
            [&](const BasicImageArtifactData& basic) { basic.PopulateArtifact(std::get<BasicImageArtifact>(imageArtifact.Artifact)); },
            [&](const CompositeImageArtifactData& composite) { composite.PopulateArtifact(std::get<CompositeImageArtifact>(imageArtifact.Artifact)); }
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

    QObject::connect(TypeComboBox, &QComboBox::currentIndexChanged, [&]() { UpdateTypeWidget(); });
}

auto ImageArtifactWidget::GetData() const noexcept -> ImageArtifactData {
    return std::visit([](auto* widget) { return ImageArtifactData { widget->GetData() }; }, TypeWidget);
}

auto ImageArtifactWidget::Populate(const ImageArtifactData& data) noexcept -> void {
    Type type = std::visit(Overload {
        [&](BasicImageArtifactData const& data) { return Type::BASIC; },
        [&](CompositeImageArtifactData const& data) { return Type::COMPOSITE; }
    }, data.Data);

    if (const int idx = TypeComboBox->findData(QVariant::fromValue(type)); idx != -1)
        TypeComboBox->setCurrentIndex(idx);

    Layout->setRowVisible(TypeComboBox, false);

    std::visit(Overload {
            [&](BasicImageArtifactWidget* widget) { return widget->Populate(std::get<BasicImageArtifactData>(data.Data)); },
            [&](CompositeImageArtifactWidget* widget) { return widget->Populate(std::get<CompositeImageArtifactData>(data.Data)); }
    }, TypeWidget);
}

auto ImageArtifactWidget::UpdateTypeWidget() noexcept -> void {
    auto type = TypeComboBox->currentData().value<Type>();

    TypeWidgetVariant newWidgetVariant = [type]() {
        switch (type) {
            case Type::BASIC:     return TypeWidgetVariant { new BasicImageArtifactWidget() };
            case Type::COMPOSITE: return TypeWidgetVariant { new CompositeImageArtifactWidget() };
        }

        return TypeWidgetVariant {};
    }();

    std::visit([&, newWidgetVariant](auto* oldSubTypeWidget) {
        std::visit([&, oldSubTypeWidget](auto* newSubTypeWidget) {
            Layout->replaceWidget(oldSubTypeWidget, newSubTypeWidget);
            delete oldSubTypeWidget;

            TypeWidget = newWidgetVariant;
        }, newWidgetVariant);
    }, TypeWidget);
}

auto ImageArtifactWidget::TypeToString(ImageArtifactWidget::Type type) noexcept -> std::string {
    switch (type) {
        case Type::BASIC:     return "Artifact";
        case Type::COMPOSITE: return "Composition";
    }

    return "";
}

auto ImageArtifactWidget::FindWidget(QWidget* widget) -> ImageArtifactWidget& {
    if (!widget)
        throw std::runtime_error("Given widget must not be nullptr");

    auto* imageArtifactWidget = widget->findChild<ImageArtifactWidget*>();

    if (!imageArtifactWidget)
        throw std::runtime_error("No image artifact widget contained in given widget");

    return *imageArtifactWidget;
}
