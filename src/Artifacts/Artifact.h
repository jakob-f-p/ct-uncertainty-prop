#pragma once

#include "../Enum.h"

#include <QLayout>
#include <QMetaObject>

#include <vtkObject.h>

struct ArtifactDetails;

class Artifact : public vtkObject {
    Q_GADGET
public:
    vtkTypeMacro(Artifact, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    std::string GetName();
    void SetName(const std::string& name);

    enum Type {
        IMAGE_ARTIFACT,
        STRUCTURE_ARTIFACT
    };
    Q_ENUM(Type);
    static std::string TypeToString(Type type);
    GET_ENUM_VALUES(Type, false);

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
    GET_ENUM_VALUES(SubType, false);

    static Artifact* NewArtifact(SubType subType);

    virtual Type GetArtifactType() const = 0;

    virtual SubType GetArtifactSubType() const = 0;

    static constexpr std::array<SubType, 8> GetImageArtifactTypes() {
        return { IMAGE_GAUSSIAN,
                 IMAGE_SALT_PEPPER,
                 IMAGE_RING,
                 IMAGE_CUPPING,
                 IMAGE_WIND_MILL,
                 IMAGE_STAIR_STEP,
                 IMAGE_STREAKING,
                 IMAGE_COMPOSITION };
    };

    static constexpr std::array<SubType, 3> GetStructureArtifactTypes(){
        return { STRUCTURE_STREAKING,
                 STRUCTURE_METALLIC,
                 STRUCTURE_MOTION };
    };

    QWidget* GetEditWidget() const;
    void SetEditWidgetData(QWidget* widget, const ArtifactDetails& artifactDetails);

    void SetData(const ArtifactDetails& artifactDetails);

    Artifact(const Artifact&) = delete;
    void operator=(const Artifact&) = delete;

protected:
    Artifact() = default;
    ~Artifact() override = default;

    ArtifactDetails GetArtifactDetails() const;

    virtual QWidget* GetChildEditWidget() const = 0;
    virtual void SetChildEditWidgetData(QWidget* widget, const ArtifactDetails& artifactDetails) const = 0;
    ArtifactDetails GetArtifactEditWidgetData(QWidget* widget) const;

    virtual void SetChildData(const ArtifactDetails& artifactDetails) = 0;

    std::string Name;

    static const QString NameLineEditObjectName;
    static const QString ChildEditWidgetObjectName;
};

struct ArtifactDetails {
    QString Name;
    Artifact::Type Type;
    Artifact::SubType SubType;

    ArtifactDetails(QString name, Artifact::Type type, Artifact::SubType subType);
    ArtifactDetails();
    virtual ~ArtifactDetails() = default;
};