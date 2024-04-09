#include "ImageArtifact.h"

#include "CompositeArtifact.h"
#include "GaussianArtifact.h"

#include <QComboBox>
#include <memory>

void ImageArtifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

Artifact::Type ImageArtifact::GetArtifactType() const {
    return Type::IMAGE_ARTIFACT;
}

ImageArtifact::ImageArtifact() :
        Parent(nullptr) {
}

CompositeArtifact* ImageArtifact::GetParent() {
    return Parent;
}

void ImageArtifact::SetParent(CompositeArtifact* parent) {
    Parent = parent;
}

bool ImageArtifact::IsComposition() const {
    return GetArtifactSubType() == SubType::IMAGE_COMPOSITION;
}

std::string ImageArtifact::GetViewName() const {
    std::string subTypeFullName = SubTypeToString(GetArtifactSubType());
    std::string subTypeViewName = subTypeFullName.erase(0, subTypeFullName.find(' ') + 1);
    std::string viewName = subTypeViewName + (Name.empty() ? "" : (" (" + Name + ")"));
    return viewName;
}



#define CONVERT_TO_IMAGE_ARTIFACT_DATA(Enum, ClassName) \
    if (variant.canConvert<ClassName##ArtifactData>())\
        return std::make_unique<ClassName##ArtifactData>(variant.value<ClassName##ArtifactData>());

#define CONVERT_TO_Q_VARIANT(Enum, ClassName) \
    case Artifact::SubType::Enum: return QVariant::fromValue(dynamic_cast<const ClassName##ArtifactData&>(data));

#define CREATE_IMAGE_ARTIFACT_DATA(Enum, ClassName) \
    case Artifact::SubType::Enum: return std::make_unique<ClassName##ArtifactData>();

std::unique_ptr<ImageArtifactData> ImageArtifactData::FromQVariant(const QVariant& variant) {
    FOR_EACH_IMAGE_ARTIFACT(CONVERT_TO_IMAGE_ARTIFACT_DATA)

    qWarning("No matching image artifact type");
    return {};
}

QVariant ImageArtifactData::ToQVariant(const ImageArtifactData& data) {
    switch (data.SubType) {
        FOR_EACH_IMAGE_ARTIFACT(CONVERT_TO_Q_VARIANT)
        default: {
            qWarning("No matching image artifact type");
            return {};
        }
    }
}

void ImageArtifactData::AddDerivedData(const ImageArtifact& artifact, ImageArtifactData& data) {
    data.ViewName = QString::fromStdString(artifact.GetViewName());
    data.AddSubTypeData(artifact);
}

void ImageArtifactData::SetDerivedData(ImageArtifact& artifact, const ImageArtifactData& data) {
    data.SetSubTypeData(artifact);
}

std::unique_ptr<ImageArtifactData> ImageArtifactData::Create(const ImageArtifact& artifact) {
    auto data = Create(artifact.GetArtifactSubType());
    return data;
}

std::unique_ptr<ImageArtifactData> ImageArtifactData::Create(Artifact::SubType subType) {
    switch (subType) {
        FOR_EACH_IMAGE_ARTIFACT(CREATE_IMAGE_ARTIFACT_DATA)
        default: {
            qWarning("No matching image artifact type");
            return {};
        }
    }
}

#undef CONVERT_TO_IMAGE_ARTIFACT_DATA
#undef CONVERT_TO_Q_VARIANT
#undef CREATE_IMAGE_ARTIFACT_DATA



#define ADD_SUB_TYPE_WIDGETS(Enum, ClassName) \
    case Artifact::SubType::Enum:             \
        return ClassName##ArtifactUi::AddSubTypeWidgets(fLayout);

#define ADD_SUB_TYPE_WIDGETS_DATA(Enum, ClassName) \
    case Artifact::SubType::Enum:                  \
        return ClassName##ArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<ClassName##ArtifactData&>(data));

#define ADD_DERIVED_WIDGETS_DATA(Enum, ClassName) \
    case Artifact::SubType::Enum:                 \
        return ClassName##ArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const ClassName##ArtifactData&>(data));

void ImageArtifactUi::AddDerivedWidgets(QFormLayout* fLayout, Artifact::SubType subType) {
    switch (subType) {
        FOR_EACH_IMAGE_ARTIFACT(ADD_SUB_TYPE_WIDGETS)
        default: qInfo("No matching image artifact type");
    }
}

void ImageArtifactUi::AddDerivedWidgetsData(QWidget* widget, ImageArtifactData& data) {
    switch (data.SubType) {
        FOR_EACH_IMAGE_ARTIFACT(ADD_SUB_TYPE_WIDGETS_DATA)
        default: qWarning("No matching image artifact type");
    }
}

void ImageArtifactUi::SetDerivedWidgetsData(QWidget* widget, const ImageArtifactData& data) {
    switch (data.SubType) {
        FOR_EACH_IMAGE_ARTIFACT(ADD_DERIVED_WIDGETS_DATA)
        default: qWarning("No matching image artifact type");
    }
}

#undef ADD_SUB_TYPE_WIDGETS
#undef ADD_SUB_TYPE_WIDGETS_DATA
#undef ADD_DERIVED_WIDGETS_DATA

std::vector<EnumString<Artifact::SubType>> ImageArtifactUi::GetSubTypeValues() {
    auto imageArtifactTypes = Artifact::GetImageArtifactTypes();
    auto enumStrings = Artifact::GetSubTypeValues();
    std::vector<EnumString<Artifact::SubType>> filtered;
    std::copy_if(enumStrings.begin(), enumStrings.end(), std::back_inserter(filtered),
                 [imageArtifactTypes](const EnumString<Artifact::SubType>& enumString) {
        auto it = std::find(imageArtifactTypes.begin(), imageArtifactTypes.end(), enumString.EnumValue);
        return it != imageArtifactTypes.end();
    });
    return filtered;
}
