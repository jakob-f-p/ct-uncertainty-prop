#include "CompositeImageArtifact.h"

#include "ImageArtifact.h"

#include "Filters/MergeParallelImageArtifactFilters.h"

#include <QComboBox>
#include <QFormLayout>

#include <vtkImageAlgorithm.h>

auto CompositeImageArtifactDetails::CompositeImageArtifactData::PopulateFromArtifact(
        const CompositeImageArtifact& artifact) noexcept -> void {

    CompositionType = artifact.CompType;
}

auto CompositeImageArtifactDetails::CompositeImageArtifactData::PopulateArtifact(
        CompositeImageArtifact& artifact) const noexcept -> void {

    artifact.CompType = CompositionType;
}



CompositeImageArtifactDetails::CompositeImageArtifactWidgetImpl::CompositeImageArtifactWidgetImpl() :
        CompositionTypeComboBox(new QComboBox()) {

    auto* fLayout = new QFormLayout(this);

    for (const auto &compositionTypeAndName : CompositeImageArtifactDetails::GetCompositionTypeValues())
        CompositionTypeComboBox->addItem(compositionTypeAndName.Name,
                                         QVariant::fromValue(compositionTypeAndName.EnumValue));

    fLayout->addRow("Composition Type", CompositionTypeComboBox);
}

auto CompositeImageArtifactDetails::CompositeImageArtifactWidgetImpl::GetData() noexcept -> CompositeImageArtifactData {
    return { CompositionTypeComboBox->currentData().value<CompositionType>()};
}

auto CompositeImageArtifactDetails::CompositeImageArtifactWidgetImpl::Populate(
        CompositeImageArtifactData const& data) noexcept -> void {

    if (int idx = CompositionTypeComboBox->findData(QVariant::fromValue(data.CompositionType));
            idx != -1)
        CompositionTypeComboBox->setCurrentIndex(idx);
}




CompositeImageArtifact::CompositeImageArtifact(CompositionType compositionType) :
        CompType(compositionType) {
}

CompositeImageArtifact::CompositeImageArtifact(const CompositeImageArtifactData& data) :
        CompositeImageArtifact(data.Data.CompositionType) {
    data.PopulateArtifact(*this);

}

CompositeImageArtifact::CompositeImageArtifact(CompositeImageArtifact&&) = default;

auto CompositeImageArtifact::operator=(CompositeImageArtifact&&) -> CompositeImageArtifact& = default;

CompositeImageArtifact::~CompositeImageArtifact() = default;

auto CompositeImageArtifact::GetViewName() const noexcept -> std::string {
    std::string const viewName = CompositionTypeToString(CompType) + (Name.empty() ? "" : (" (" + Name + ")"));
    return viewName;
}

auto CompositeImageArtifact::GetProperties() noexcept -> PipelineParameterProperties {
    return {};
}

auto CompositeImageArtifact::ContainsImageArtifact(const ImageArtifact& imageArtifact) const noexcept -> bool {
    return std::any_of(ImageArtifacts.begin(), ImageArtifacts.end(), [&](auto& child) {
        if (child.get() == &imageArtifact)
            return true;

        return child->ContainsImageArtifact(imageArtifact);
    });
}

auto CompositeImageArtifact::AddImageArtifact(ImageArtifact&& artifact, int idx) -> ImageArtifact& {
    if (idx == -1)
        return *ImageArtifacts.emplace_back(std::make_unique<ImageArtifact>(std::move(artifact)));

    return *ImageArtifacts.emplace(std::next(ImageArtifacts.begin(), idx),
                                   std::make_unique<ImageArtifact>(std::move(artifact)))->get();
}

void CompositeImageArtifact::RemoveImageArtifact(const ImageArtifact& imageArtifact) {
    auto artifactIt = std::find_if(ImageArtifacts.begin(), ImageArtifacts.end(),
                                   [&](auto& artifact) { return artifact.get() == &imageArtifact; });

    if (artifactIt == ImageArtifacts.end())
        throw std::runtime_error("Cannot remove image artifact from this composition since"
                                 "given image artifact is not contained in child artifacts");

    ImageArtifacts.erase(artifactIt);
}

void CompositeImageArtifact::MoveChildImageArtifact(const ImageArtifact& imageArtifact, int newIdx) {
    if (newIdx < 0 || newIdx >= ImageArtifacts.size())
        throw std::runtime_error("Cannot move given image artifact to index");

    auto currentIt = std::find_if(ImageArtifacts.begin(), ImageArtifacts.end(),
                                  [&](auto& artifact) { return artifact.get() == &imageArtifact; });
    if (currentIt == ImageArtifacts.end())
        throw std::runtime_error("Cannot move given image artifact to index");

    auto currentIdx = std::distance(ImageArtifacts.begin(), currentIt);

    if (currentIdx == newIdx)
        return;

    auto newIt = std::next(ImageArtifacts.begin(), newIdx);
    if (currentIdx < newIdx) {
        std::rotate(currentIt, std::next(currentIt), std::next(newIt));
    } else {
        std::rotate(newIt, std::next(newIt), std::next(currentIt));
    }
}

auto CompositeImageArtifact::NumberOfChildren() const noexcept -> uint8_t {
    return static_cast<uint8_t >(ImageArtifacts.size());
}

auto CompositeImageArtifact::ChildArtifact(uint8_t idx) const -> ImageArtifact& {
    if (idx >= ImageArtifacts.size())
        throw std::runtime_error("Given child index too large");

    return *ImageArtifacts[idx];
}

auto CompositeImageArtifact::GetChildIdx(const ImageArtifact& imageArtifact) const -> uint8_t {
    uint64_t const idx = std::distance(ImageArtifacts.begin(),
                                 std::find_if(ImageArtifacts.begin(), ImageArtifacts.end(),
                                              [&](auto& artifact) { return artifact.get() == &imageArtifact; }));
    return static_cast<uint8_t>(idx);
}

auto CompositeImageArtifact::Get(uint16_t targetIdx, uint16_t& currentIdx) -> ImageArtifact* {
    for (const auto& childArtifact : ImageArtifacts) {
        ImageArtifact* artifact = childArtifact->Get(targetIdx, currentIdx);

        if (artifact)
            return artifact;
    }

    return nullptr;
}

auto CompositeImageArtifact::IndexOf(const ImageArtifact& imageArtifact, uint16_t& currentIdx) const -> int32_t {
    for (const auto& childArtifact : ImageArtifacts) {
        int32_t idx = childArtifact->IndexOf(imageArtifact, currentIdx);

        if (idx != -1)
            return idx;
    }

    return -1;
}

auto CompositeImageArtifact::AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm& {
    switch (CompType) {

        case CompositionType::SEQUENTIAL: {
            vtkImageAlgorithm* currentImageAlgorithm = &inputAlgorithm;

            for (auto& imageArtifact : ImageArtifacts)
                currentImageAlgorithm = &imageArtifact->AppendImageFilters(*currentImageAlgorithm);

            return *currentImageAlgorithm;
        }

        case CompositionType::PARALLEL: {
            Filter->RemoveAllInputConnections(1);

            Filter->SetBaseFilterConnection(inputAlgorithm.GetOutputPort());

            for (auto& imageArtifact : ImageArtifacts) {
                auto& appendedAlgorithm = imageArtifact->AppendImageFilters(inputAlgorithm);

                Filter->AddParallelFilterConnection(appendedAlgorithm.GetOutputPort());
            }

            return *Filter;
        }

        default: throw std::runtime_error("invalid composition type");
    }
}

