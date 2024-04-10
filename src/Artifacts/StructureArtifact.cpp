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



#define FOR_EACH_STRUCTURE_ARTIFACT(DO) \
    DO(STRUCTURE_MOTION, Motion)        \
//    DO(STRUCTURE_STREAKING, Streaking)  \
//    DO(STRUCTURE_METALLIC, Metallic)    \


#define CONVERT_TO_STRUCTURE_ARTIFACT_DATA(Enum, Type) \
    if (variant.canConvert<ARTIFACT_DATA_TYPE(Type)>())\
        return std::make_unique<ARTIFACT_DATA_TYPE(Type)>(variant.value<ARTIFACT_DATA_TYPE(Type)>());

#define CONVERT_TO_Q_VARIANT(Enum, Type) \
    case Artifact::SubType::Enum: return QVariant::fromValue(dynamic_cast<const ARTIFACT_DATA_TYPE(Type)&>(data));

#define CREATE_STRUCTURE_ARTIFACT_DATA(Enum, Type) \
    case Artifact::SubType::Enum: return std::make_unique<ARTIFACT_DATA_TYPE(Type)>();

std::unique_ptr<StructureArtifactData> StructureArtifactData::QVariantToData(const QVariant& variant) {
    FOR_EACH_STRUCTURE_ARTIFACT(CONVERT_TO_STRUCTURE_ARTIFACT_DATA)

    qWarning("No matching structure artifact type");
    return {};
}

QVariant StructureArtifactData::DataToQVariant(const StructureArtifactData& data) {
    switch (data.SubType) {
        FOR_EACH_STRUCTURE_ARTIFACT(CONVERT_TO_Q_VARIANT)
        default: {
            qWarning("No matching structure artifact type");
            return {};
        }
    }
}

void StructureArtifactData::AddDerivedData(const StructureArtifact& artifact, StructureArtifactData& data) {
    data.AddSubTypeData(artifact);
}

void StructureArtifactData::SetDerivedData(StructureArtifact& artifact, const StructureArtifactData& data) {
    data.SetSubTypeData(artifact);
}

std::unique_ptr<StructureArtifactData> StructureArtifactData::Create(Artifact::SubType subType) {
    switch (subType) {
        FOR_EACH_STRUCTURE_ARTIFACT(CREATE_STRUCTURE_ARTIFACT_DATA)
        default: {
            qWarning("No matching structure artifact type");
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

void StructureArtifactUi::AddDerivedWidgets(QFormLayout* fLayout, Artifact::SubType subType) {
    switch (subType) {
        FOR_EACH_STRUCTURE_ARTIFACT(ADD_SUB_TYPE_WIDGETS)
        default: qInfo("No matching structure artifact type");
    }
}

void StructureArtifactUi::AddDerivedWidgetsData(QWidget* widget, StructureArtifactData& data) {
    switch (data.SubType) {
        FOR_EACH_STRUCTURE_ARTIFACT(ADD_SUB_TYPE_WIDGETS_DATA)
        default: qWarning("No matching structure artifact type");
    }
}

void StructureArtifactUi::SetDerivedWidgetsData(QWidget* widget, const StructureArtifactData& data) {
    switch (data.SubType) {
        FOR_EACH_STRUCTURE_ARTIFACT(ADD_DERIVED_WIDGETS_DATA)
        default: qWarning("No matching structure artifact type");
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

#undef ADD_SUB_TYPE_WIDGETS
#undef ADD_SUB_TYPE_WIDGETS_DATA
#undef ADD_DERIVED_WIDGETS_DATA

#undef FOR_EACH_STRUCTURE_ARTIFACT
