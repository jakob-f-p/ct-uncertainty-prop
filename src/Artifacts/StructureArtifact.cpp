#include "StructureArtifact.h"

#include "MotionArtifact.h"

void StructureArtifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

Artifact::Type StructureArtifact::GetArtifactType() const {
    return Type::STRUCTURE_ARTIFACT;
}

StructureArtifact* StructureArtifact::NewStructureArtifact(Artifact::SubType subType) {
    switch (subType) {
        case SubType::STRUCTURE_STREAKING: qWarning("implement");
        case SubType::STRUCTURE_METALLIC: qWarning("implement");
        case SubType::STRUCTURE_MOTION: return MotionArtifact::New();
        default: {
            qWarning("Not a supported Structure artifact subtype");
            return {};
        }
    }
}

void StructureArtifact::DeepCopy(StructureArtifact* source) {
    Name = source->Name;
}



void StructureArtifactData::AddDerivedData(const StructureArtifact& artifact, StructureArtifactData& data) {
    data.AddSubTypeData(artifact);
}

void StructureArtifactData::SetDerivedData(StructureArtifact& artifact, const StructureArtifactData& data) {
    data.SetSubTypeData(artifact);
}

std::unique_ptr<StructureArtifactData> StructureArtifactData::Create(const StructureArtifact& artifact) {
    auto data = Create(artifact.GetArtifactSubType());
    return data;
}

std::unique_ptr<StructureArtifactData> StructureArtifactData::Create(Artifact::SubType subType) {
    switch (subType) {
//        case Artifact::SubType::STRUCTURE_STREAKING:
//            return std::make_unique<StreakingArtifactData>(StreakingArtifactData{});
//        case Artifact::SubType::STRUCTURE_METALLIC:
//            return std::make_unique<MetallicArtifactData>(MetallicArtifactData{});
        case Artifact::SubType::STRUCTURE_MOTION:
            return std::make_unique<MotionArtifactData>(MotionArtifactData{});
        default: {
            qWarning("Not a supported Structure artifact subtype");
            return {};
        }
    }
}

std::unique_ptr<StructureArtifactData> StructureArtifactData::FromQVariant(const QVariant& variant) {
//    if (variant.canConvert<StreakingArtifactData>())
//        return std::make_unique<StreakingArtifactData>(variant.value<StreakingArtifactData>(variant));
//    if (variant.canConvert<MetallicArtifactData>())
//        return std::make_unique<MetallicArtifactData>(variant.value<MetallicArtifactData>(variant));
    if (variant.canConvert<MotionArtifactData>())
        return std::make_unique<MotionArtifactData>(variant.value<MotionArtifactData>());

    qWarning("Not a supported Structure artifact subtype");
    return {};
}

void StructureArtifactUi::AddDerivedWidgets(QFormLayout* fLayout, Artifact::SubType subType) {
    switch (subType) {
//        case Artifact::SubType::STRUCTURE_STREAKING: return StreakingArtifactUi::AddSubTypeWidgets(fLayout);
//        case Artifact::SubType::STRUCTURE_METALLIC:  return MetallicArtifactUi::AddSubTypeWidgets(fLayout);
        case Artifact::SubType::STRUCTURE_MOTION:    return MotionArtifactUi::AddSubTypeWidgets(fLayout);
        default: qInfo("Not a supported Structure artifact subtype");
    }
}

void StructureArtifactUi::AddDerivedWidgetsData(QWidget* widget, StructureArtifactData& data) {
    switch (data.SubType) {
//        case Artifact::SubType::STRUCTURE_STREAKING:
//            return StreakingArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<StreakingArtifactData&>(rData));
//        case Artifact::SubType::STRUCTURE_METALLIC:
//            return MetallicArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<MetallicArtifactData&>(rData));
        case Artifact::SubType::STRUCTURE_MOTION:
            return MotionArtifactUi::AddSubTypeWidgetsData(widget, dynamic_cast<MotionArtifactData&>(data));
        default: qWarning("Not a supported Structure artifact subtype");
    }
}

void StructureArtifactUi::SetDerivedWidgetsData(QWidget* widget, const StructureArtifactData& data) {
    switch (data.SubType) {
//        case Artifact::SubType::STRUCTURE_STREAKING:
//            return StreakingArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const StreakingArtifactData&>(rData));
//        case Artifact::SubType::STRUCTURE_METALLIC:
//            return MetallicArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const MetallicArtifactData&>(rData));
        case Artifact::SubType::STRUCTURE_MOTION:
            return MotionArtifactUi::SetSubTypeWidgetsData(widget, dynamic_cast<const MotionArtifactData&>(data));
        default: qWarning("Not a supported Structure artifact subtype");
    }
}

std::vector<EnumString<Artifact::SubType>> StructureArtifactUi::GetSubTypeValues() {
    auto structureArtifactTypes = Artifact::GetStructureArtifactTypes();
    auto enumStrings = Artifact::GetSubTypeValues();
    std::vector<EnumString<Artifact::SubType>> filtered;
    std::copy_if(enumStrings.begin(), enumStrings.end(), std::back_inserter(filtered),
                 [structureArtifactTypes](const EnumString<Artifact::SubType>& enumString) {
        auto it = std::find(structureArtifactTypes.begin(), structureArtifactTypes.end(), enumString.EnumValue);
        return it != structureArtifactTypes.end();
    });
    return filtered;
}
