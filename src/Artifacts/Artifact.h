#pragma once

#include "../Enum.h"

#include <QMetaObject>

#include <vtkObject.h>

#include <array>
#include <string>

struct ArtifactDetails;

class Artifact : public vtkObject {
    Q_GADGET
public:
    vtkTypeMacro(Artifact, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    std::string GetName();

    enum Type {
        IMAGE_ARTIFACT,
        STRUCTURE_ARTIFACT
    };
    Q_ENUM(Type);
    static std::string TypeToString(Type type);
    GET_ENUM_VALUES(Type);

    enum SubType {
        IMAGE_GAUSSIAN,
        IMAGE_SALT_PEPPER,
        IMAGE_RING,
        IMAGE_CUPPING,
        IMAGE_WIND_MILL,
        IMAGE_STAIR_STEP,
        IMAGE_STREAKING,
        IMAGE_COMPOSITION,

        STRUCTURE_STREAKING,
        STRUCTURE_METALLIC,
        STRUCTURE_MOTION
    };
    Q_ENUM(SubType);
    static std::string SubTypeToString(SubType subType);
    GET_ENUM_VALUES(SubType);

    virtual Type GetArtifactType() const = 0;

    virtual SubType GetArtifactSubType() const = 0;

    static constexpr std::array<SubType, 7> GetImageArtifactTypes() {
        return { IMAGE_GAUSSIAN,
                 IMAGE_SALT_PEPPER,
                 IMAGE_RING,
                 IMAGE_CUPPING,
                 IMAGE_WIND_MILL,
                 IMAGE_STAIR_STEP,
                 IMAGE_STREAKING };
    };

    static constexpr std::array<SubType, 3> GetStructureArtifactTypes(){
        return { STRUCTURE_STREAKING,
                 STRUCTURE_METALLIC,
                 STRUCTURE_MOTION };
    };

    Artifact(const Artifact&) = delete;
    void operator=(const Artifact&) = delete;

protected:
    Artifact() = default;
    ~Artifact() override = default;

    ArtifactDetails GetArtifactDetails();

    std::string Name;
};

struct ArtifactDetails {
    QString Name;
    Artifact::Type Type;
    Artifact::SubType SubType;
};