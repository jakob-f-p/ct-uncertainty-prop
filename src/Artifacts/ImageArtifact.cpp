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



#define FOR_EACH_IMAGE_ARTIFACT(DO)   \
    DO(IMAGE_GAUSSIAN, Gaussian)      \
    DO(IMAGE_COMPOSITION, Composite)  \
//    DO(IMAGE_SALT_PEPPER, SaltPepper) \
//    DO(IMAGE_RING, Ring)              \
//    DO(IMAGE_CUPPING, Cupping)        \
//    DO(IMAGE_WIND_MILL, WindMill)     \
//    DO(IMAGE_STAIR_STEP, StairStep)   \
//    DO(IMAGE_STREAKING, Streaking)    \


#define CONVERT_TO_IMAGE_ARTIFACT_DATA(Enum, Type) \
    if (variant.canConvert<ARTIFACT_DATA_TYPE(Type)>())\
        return std::make_unique<ARTIFACT_DATA_TYPE(Type)>(variant.value<ARTIFACT_DATA_TYPE(Type)>());

#define CONVERT_TO_Q_VARIANT(Enum, Type) \
    case Artifact::SubType::Enum: return QVariant::fromValue(dynamic_cast<const ARTIFACT_DATA_TYPE(Type)&>(data));

#define CREATE_IMAGE_ARTIFACT_DATA(Enum, Type) \
    case Artifact::SubType::Enum: return std::make_unique<ARTIFACT_DATA_TYPE(Type)>();

auto ImageArtifactData::QVariantToData(const QVariant& variant) -> std::unique_ptr<ImageArtifactData> {
    FOR_EACH_IMAGE_ARTIFACT(CONVERT_TO_IMAGE_ARTIFACT_DATA)

    qWarning("No matching image artifact type");
    return {};
}

QVariant ImageArtifactData::DataToQVariant(const ImageArtifactData& data) {
    switch (data.SubType) {
        FOR_EACH_IMAGE_ARTIFACT(CONVERT_TO_Q_VARIANT)
        default: {
            qWarning("No matching image artifact type");
            return {};
        }
    }
}

void ImageArtifactData::AddDerivedData(const ImageArtifact& artifact, ImageArtifactData& data) {
    data.AddSubTypeData(artifact);
}

void ImageArtifactData::SetDerivedData(ImageArtifact& artifact, const ImageArtifactData& data) {
    data.SetSubTypeData(artifact);
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



#define ADD_SUB_TYPE_WIDGETS(Enum, Type) \
    case Artifact::SubType::Enum:             \
    return ARTIFACT_UI_TYPE(Type)::AddSubTypeWidgets(fLayout);

#define ADD_SUB_TYPE_WIDGETS_DATA(Enum, Type) \
    case Artifact::SubType::Enum:                  \
        return ARTIFACT_UI_TYPE(Type)::AddSubTypeWidgetsData(widget, dynamic_cast<ARTIFACT_DATA_TYPE(Type)&>(data));

#define ADD_DERIVED_WIDGETS_DATA(Enum, Type) \
    case Artifact::SubType::Enum:                 \
        return ARTIFACT_UI_TYPE(Type)::SetSubTypeWidgetsData(widget, dynamic_cast<const ARTIFACT_DATA_TYPE(Type)&>(data));

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

#undef ADD_SUB_TYPE_WIDGETS
#undef ADD_SUB_TYPE_WIDGETS_DATA
#undef ADD_DERIVED_WIDGETS_DATA

#undef FOR_EACH_IMAGE_ARTIFACT