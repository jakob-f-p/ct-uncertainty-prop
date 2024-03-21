#pragma once

#include <vtkObject.h>

#include <array>
#include <string>

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

    std::string Name;
};