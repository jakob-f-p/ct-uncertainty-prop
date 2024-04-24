#pragma once

#include "../Enum.h"

#include <QWidget>

class CompositeImageArtifact;
class ImageArtifact;
class NameLineEdit;

class QComboBox;
class QFormLayout;
class QGroupBox;

class vtkImageAlgorithm;

namespace ImageArtifactBaseDetails {

    template<typename T>
    concept TArtifactData = requires(T data, T::Artifact artifact) {
        data.PopulateFromArtifact(artifact);
        data.PopulateArtifact(artifact);
    };


    class ImageArtifactBase {
    public:
        [[nodiscard]] auto
        GetName() const noexcept -> std::string { return Name; }

        auto
        SetName(const std::string& name) noexcept -> void { Name = name; }

        [[nodiscard]] auto
        GetParent() const noexcept -> ImageArtifact* { return Parent; }

        auto
        SetParent(ImageArtifact* parent) noexcept -> void { Parent = parent; }

        auto
        AppendImageFilters(vtkImageAlgorithm& inputAlgorithm) -> vtkImageAlgorithm&;

    protected:
        friend class ::CompositeImageArtifact;
        template<TArtifactData ArtifactData> friend struct ImageArtifactBaseData;

        ImageArtifactBase() = default;

        std::string Name;
        ImageArtifact* Parent = nullptr;
    };


    template<TArtifactData ArtifactData>
    struct ImageArtifactBaseData {
        QString Name;
        QString ViewName;
        ArtifactData Data;

        using Artifact = ArtifactData::Artifact;

        auto
        PopulateFromArtifact(const Artifact& artifact) noexcept -> void;

        auto
        PopulateArtifact(Artifact& artifact) const noexcept -> void;
    };


    template<typename T>
    concept TArtifactWidget = std::derived_from<T, QWidget>
                              && requires(T widget, T::Data data) {
        { widget.GetData() } -> std::same_as<typename T::Data>;
        widget.Populate(data);
    };

    template<TArtifactWidget ArtifactWidget, typename ArtifactData>
    class ImageArtifactBaseWidget : public QWidget {
    public:
        ImageArtifactBaseWidget();

        [[nodiscard]] auto
        GetData() const noexcept -> ArtifactData;

        auto
        Populate(const ArtifactData& data) noexcept -> void;

    private:
        QFormLayout* Layout;
        NameLineEdit* NameEdit;
        ArtifactWidget* ArtifactEditWidget;
    };

}

