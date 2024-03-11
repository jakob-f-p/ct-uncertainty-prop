#pragma once

#include <string>
#include <vector>

#include <vtkObject.h>

class Artifact : public vtkObject {
public:
    vtkTypeMacro(Artifact, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    std::string GetName();

    enum Type {
        IMAGE_ARTIFACT,
        STRUCTURE_ARTIFACT
    };

    enum SubType {
        IMAGE_GAUSSIAN,
        IMAGE_SALT_PEPPER,
        IMAGE_RING,
        IMAGE_CUPPING,
        IMAGE_WIND_MILL,
        IMAGE_STAIR_STEP,
        IMAGE_STREAKING,

        STRUCTURE_STREAKING,
        STRUCTURE_METALLIC,
        STRUCTURE_MOTION
    };

    virtual Type GetArtifactType() const = 0;

    virtual SubType GetArtifactSubType() const = 0;

    static std::vector<SubType> GetImageArtifactTypes();

    static std::vector<SubType> GetStructureArtifactTypes();

    Artifact(const Artifact&) = delete;
    void operator=(const Artifact&) = delete;

protected:
    Artifact() = default;
    ~Artifact() override = default;

    std::string Name;
};