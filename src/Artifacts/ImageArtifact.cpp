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



std::unique_ptr<ImageArtifactData> ImageArtifactData::FromQVariant(const QVariant& variant) {
    if (variant.canConvert<GaussianArtifactData>())
        return std::make_unique<GaussianArtifactData>(variant.value<GaussianArtifactData>());
//    if (variant.canConvert<SaltPepperArtifactData>())
//        return std::make_unique<SaltPepperArtifactData>(variant.value<SaltPepperArtifactData>());
//    if (variant.canConvert<RingArtifactData>())
//        return std::make_unique<RingArtifactData>(variant.value<RingArtifactData>());
//    if (variant.canConvert<CuppingArtifactData>())
//        return std::make_unique<CuppingArtifactData>(variant.value<CuppingArtifactData>());
//    if (variant.canConvert<WindmillArtifactData>())
//        return std::make_unique<WindmillArtifactData>(variant.value<WindmillArtifactData>());
//    if (variant.canConvert<StairStepArtifactData>())
//        return std::make_unique<StairStepArtifactData>(variant.value<StairStepArtifactData>());
//    if (variant.canConvert<StreakingArtifactData>())
//        return std::make_unique<StreakingArtifactData>(variant.value<StreakingArtifactData>());
    if (variant.canConvert<CompositeArtifactData>())
        return std::make_unique<CompositeArtifactData>(variant.value<CompositeArtifactData>());
//    if (variant.canConvert<ImageArtifactData>())
//        return std::make_unique<ImageArtifactData>(variant.value<ImageArtifactData>());

    qWarning("No matching image artifact type");
    return {};
}

QVariant ImageArtifactData::ToQVariant(const ImageArtifactData& data) {
    switch (data.SubType) {
        case Artifact::SubType::IMAGE_GAUSSIAN:
            return QVariant::fromValue(dynamic_cast<const GaussianArtifactData&>(data));
//        case Artifact::SubType::IMAGE_SALT_PEPPER:
//            return QVariant::fromValue(dynamic_cast<const SaltPepperArtifactData&>(data));
//        case Artifact::SubType::IMAGE_RING:
//            return QVariant::fromValue(dynamic_cast<const RingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_CUPPING:
//            return QVariant::fromValue(dynamic_cast<const CuppingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_WIND_MILL:
//            return QVariant::fromValue(dynamic_cast<const WindmillArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STAIR_STEP:
//            return QVariant::fromValue(dynamic_cast<const StairStepArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STREAKING:
//            return QVariant::fromValue(dynamic_cast<const StreakingArtifactData&>(data));
        case Artifact::SubType::IMAGE_COMPOSITION:
            return QVariant::fromValue(dynamic_cast<const CompositeArtifactData&>(data));
        default: {
            qWarning("No matching image artifact type");
            return {};
        }
    }

    qWarning("No matching image artifact type");
    return {};
}

void ImageArtifactData::AddDerivedData(const ImageArtifact& artifact, ImageArtifactData& data) {
    data.ViewName = QString::fromStdString(artifact.GetViewName());

    switch (artifact.GetArtifactSubType()) {
        case Artifact::SubType::IMAGE_GAUSSIAN: {
            return GaussianArtifactData::AddSubTypeData(dynamic_cast<const GaussianArtifact&>(artifact),
                                                        dynamic_cast<GaussianArtifactData&>(data));
        }
//        case Artifact::SubType::IMAGE_SALT_PEPPER:
//            return SaltPepperArtifactData::AddSubTypeData(dynamic_cast<const SaltPepperArtifact&>(artifact),
//                                                          dynamic_cast<SaltPepperArtifactData&>(data));
//        case Artifact::SubType::IMAGE_RING:
//            return RingArtifactData::AddSubTypeData(dynamic_cast<const RingArtifact&>(artifact),
//                                                    dynamic_cast<RingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_CUPPING:
//            return CuppingArtifactData::AddSubTypeData(dynamic_cast<const CuppingArtifact&>(artifact),
//                                                       dynamic_cast<CuppingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_WIND_MILL:
//            return WindmillArtifactData::AddSubTypeData(dynamic_cast<const WindmillArtifact&>(artifact),
//                                                        dynamic_cast<WindmillArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STAIR_STEP:
//            return StairStepArtifactData::AddSubTypeData(dynamic_cast<const StairStepArtifact&>(artifact),
//                                                         dynamic_cast<StairStepArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STREAKING:
//            return StreakingArtifactData::AddSubTypeData(dynamic_cast<const StreakingArtifact&>(artifact),
//                                                         dynamic_cast<StreakingArtifactData&>(data));
        case Artifact::SubType::IMAGE_COMPOSITION:
            return CompositeArtifactData::AddSubTypeData(dynamic_cast<const CompositeArtifact&>(artifact),
                                                         dynamic_cast<CompositeArtifactData&>(data));
        default:
            qWarning("No matching image artifact type");
    }
}

void ImageArtifactData::SetDerivedData(ImageArtifact& artifact, const ImageArtifactData& data) {
    switch (artifact.GetArtifactSubType()) {
        case Artifact::SubType::IMAGE_GAUSSIAN:
            return GaussianArtifactData::SetSubTypeData(dynamic_cast<GaussianArtifact&>(artifact),
                                                        dynamic_cast<const GaussianArtifactData&>(data));
//        case Artifact::SubType::IMAGE_SALT_PEPPER:
//            return SaltPepperArtifactData::SetSubTypeData(dynamic_cast<SaltPepperArtifact&>(artifact),
//                                                          dynamic_cast<const SaltPepperArtifactData&>(data));
//        case Artifact::SubType::IMAGE_RING:
//            return RingArtifactData::SetSubTypeData(dynamic_cast<RingArtifact&>(artifact),
//                                                    dynamic_cast<const RingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_CUPPING:
//            return CuppingArtifactData::SetSubTypeData(dynamic_cast<CuppingArtifact&>(artifact),
//                                                       dynamic_cast<const CuppingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_WIND_MILL:
//            return WindmillArtifactData::SetSubTypeData(dynamic_cast<WindmillArtifact&>(artifact),
//                                                        dynamic_cast<const WindmillArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STAIR_STEP:
//            return StairStepArtifactData::SetSubTypeData(dynamic_cast<StairStepArtifact&>(artifact),
//                                                         dynamic_cast<const StairStepArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STREAKING:
//            return StreakingArtifactData::SetSubTypeData(dynamic_cast<StreakingArtifact&>(artifact),
//                                                         dynamic_cast<const StreakingArtifactData&>(data));
        case Artifact::SubType::IMAGE_COMPOSITION:
            return CompositeArtifactData::SetSubTypeData(dynamic_cast<CompositeArtifact&>(artifact),
                                                         dynamic_cast<const CompositeArtifactData&>(data));
        default:
            qWarning("No matching image artifact type");
    }
}

std::unique_ptr<ImageArtifactData> ImageArtifactData::GetEmpty(const ImageArtifact& artifact) {
    auto data = GetEmpty(artifact.GetArtifactSubType());
    return data;
}

std::unique_ptr<ImageArtifactData> ImageArtifactData::GetEmpty(Artifact::SubType subType) {
    switch (subType) {
        case Artifact::SubType::IMAGE_GAUSSIAN:
            return std::make_unique<GaussianArtifactData>();
//        case Artifact::SubType::IMAGE_SALT_PEPPER:
//            return std::make_unique<SaltPepperArtifactData>(SaltPepperArtifactData{});
//        case Artifact::SubType::IMAGE_RING:
//            return std::make_unique<RingArtifactData>(RingArtifactData{});
//        case Artifact::SubType::IMAGE_CUPPING:
//            return std::make_unique<CuppingArtifactData>(CuppingArtifactData{});
//        case Artifact::SubType::IMAGE_WIND_MILL:
//            return std::make_unique<WindmillArtifactData>(WindmillArtifactData{});
//        case Artifact::SubType::IMAGE_STAIR_STEP:
//            return std::make_unique<StairStepArtifactData>(StairStepArtifactData{});
//        case Artifact::SubType::IMAGE_STREAKING:
//            return std::make_unique<StreakingArtifactData>(StreakingArtifactData{});
        case Artifact::SubType::IMAGE_COMPOSITION:
            return std::make_unique<CompositeArtifactData>(CompositeArtifactData{});
        default: {
            qWarning("No matching image artifact type");
            return {};
        }
    }
}


void ImageArtifactUi::AddDerivedWidgets(QFormLayout* fLayout, Artifact::SubType subType) {
    switch (subType) {
        case Artifact::SubType::IMAGE_GAUSSIAN:    return GaussianArtifactUi::AddSubTypeWidgets(fLayout);
//        case Artifact::SubType::IMAGE_SALT_PEPPER: return SaltPepperArtifactUi::AddSubTypeWidgets(fLayout);
//        case Artifact::SubType::IMAGE_RING:        return RingArtifactUi::AddSubTypeWidgets(fLayout);
//        case Artifact::SubType::IMAGE_CUPPING:     return CuppingArtifactUi::AddSubTypeWidgets(fLayout);
//        case Artifact::SubType::IMAGE_WIND_MILL:   return WindmillArtifactUi::AddSubTypeWidgets(fLayout);
//        case Artifact::SubType::IMAGE_STAIR_STEP:  return StairStepArtifactUi::AddSubTypeWidgets(fLayout);
//        case Artifact::SubType::IMAGE_STREAKING:   return StreakingArtifactUi::AddSubTypeWidgets(fLayout);
        case Artifact::SubType::IMAGE_COMPOSITION: return CompositeArtifactUi::AddSubTypeWidgets(fLayout);
        default: qInfo("No matching image artifact type");
    }
}

void ImageArtifactUi::AddDerivedWidgetsData(QWidget* widget, ImageArtifactData& data) {
    switch (data.SubType) {
        case Artifact::SubType::IMAGE_GAUSSIAN:
            return GaussianArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<GaussianArtifactData&>(data));
//        case Artifact::SubType::IMAGE_SALT_PEPPER:
//            return SaltPepperArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<SaltPepperArtifactData&>(data));
//        case Artifact::SubType::IMAGE_RING:
//            return RingArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<RingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_CUPPING:
//            return CuppingArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<CuppingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_WIND_MILL:
//            return WindmillArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<WindmillArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STAIR_STEP:
//            return StairStepArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<StairStepArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STREAKING:
//            return StreakingArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<StreakingArtifactData&>(data));
        case Artifact::SubType::IMAGE_COMPOSITION:
            return CompositeArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<CompositeArtifactData&>(data));
        default: qWarning("No matching image artifact type");
    }
}

void ImageArtifactUi::SetDerivedWidgetsData(QWidget* widget, const ImageArtifactData& data) {
    switch (data.SubType) {
        case Artifact::SubType::IMAGE_GAUSSIAN:
            return GaussianArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const GaussianArtifactData&>(data));
//        case Artifact::SubType::IMAGE_SALT_PEPPER:
//            return SaltPepperArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const SaltPepperArtifactData&>(data));
//        case Artifact::SubType::IMAGE_RING:
//            return RingArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const RingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_CUPPING:
//            return CuppingArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const CuppingArtifactData&>(data));
//        case Artifact::SubType::IMAGE_WIND_MILL:
//            return WindmillArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const WindmillArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STAIR_STEP:
//            return StairStepArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const StairStepArtifactData&>(data));
//        case Artifact::SubType::IMAGE_STREAKING:
//            return StreakingArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const StreakingArtifactData&>(data));
        case Artifact::SubType::IMAGE_COMPOSITION:
            return CompositeArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const CompositeArtifactData&>(data));
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
