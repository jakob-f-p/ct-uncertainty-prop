#pragma once

#include "../Enum.h"

#include <QLayout>
#include <QMetaObject>

#include <vtkObject.h>

class Artifact : public vtkObject {
    Q_GADGET
public:
    vtkTypeMacro(Artifact, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    void SetName(const std::string& name);

    enum struct Type {
        IMAGE_ARTIFACT,
        STRUCTURE_ARTIFACT,
        INVALID
    };
    Q_ENUM(Type);
    static std::string TypeToString(Type type);
    ENUM_GET_VALUES(Type, true);

    enum struct SubType {
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
        STRUCTURE_MOTION,

        INVALID
    };
    Q_ENUM(SubType);
    static std::string SubTypeToString(SubType subType);
    ENUM_GET_VALUES(SubType, true);
    static Type GetType(SubType subType);

    static Artifact* NewArtifact(SubType subType);

    virtual Type GetArtifactType() const = 0;

    virtual SubType GetArtifactSubType() const = 0;

    static constexpr std::array<SubType, 8> GetImageArtifactTypes() {
        return { SubType::IMAGE_GAUSSIAN,
                 SubType::IMAGE_SALT_PEPPER,
                 SubType::IMAGE_RING,
                 SubType::IMAGE_CUPPING,
                 SubType::IMAGE_WIND_MILL,
                 SubType::IMAGE_STAIR_STEP,
                 SubType::IMAGE_STREAKING,
                 SubType::IMAGE_COMPOSITION };
    };

    static constexpr std::array<SubType, 3> GetStructureArtifactTypes(){
        return { SubType::STRUCTURE_STREAKING,
                 SubType::STRUCTURE_METALLIC,
                 SubType::STRUCTURE_MOTION };
    };

    std::string GetViewName() const;

    Artifact(const Artifact&) = delete;
    void operator=(const Artifact&) = delete;

protected:
    Artifact() = default;
    ~Artifact() override = default;

    template<typename TArtifact, typename Data> friend struct ArtifactData;

    std::string Name;
};


template<typename TArtifact, typename TData>
struct ArtifactData {
    QString Name;
    QString ViewName;
    Artifact::Type Type = Artifact::Type::INVALID;
    Artifact::SubType SubType = Artifact::SubType::INVALID;

    static std::unique_ptr<TData> FromQVariant(const QVariant& variant);

    static QVariant ToQVariant(const TData& data);

    static std::unique_ptr<TData> GetData(const TArtifact& artifact);

    static void SetData(TArtifact& artifact, const TData& data);

    static void SetData(TArtifact& artifact, const QVariant& variant);

protected:
    ArtifactData() = default;
    ~ArtifactData() = default;

    static std::unique_ptr<TData> Create(const TArtifact& artifact);

    virtual void AddSubTypeData(const TArtifact& artifact) = 0;

    virtual void SetSubTypeData(TArtifact& artifact) const = 0;
};



template<typename Ui, typename Data>
class ArtifactUi {
public:
    static QWidget* GetWidget(bool showSubTypeComboBox = false);

    static std::unique_ptr<Data> GetWidgetData(QWidget* widget);

    static void SetWidgetData(QWidget* widget, const Data& data);

private:
    static const QString NameEditObjectName;
    static const QString TypeComboBoxObjectName;
    static const QString SubTypeComboBoxObjectName;
    static const QString SubTypeWidgetName;
};


#define ARTIFACT_DATA_TYPE(ArtifactType) \
    ArtifactType##ArtifactData

#define ARTIFACT_UI_TYPE(ArtifactType) \
    ArtifactType##ArtifactUi
