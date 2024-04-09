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

    enum class Type {
        IMAGE_ARTIFACT,
        STRUCTURE_ARTIFACT,
        INVALID
    };
    Q_ENUM(Type);
    static std::string TypeToString(Type type);
    GET_ENUM_VALUES(Type, true);

    enum class SubType {
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
    GET_ENUM_VALUES(SubType, true);
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

    Artifact(const Artifact&) = delete;
    void operator=(const Artifact&) = delete;

protected:
    Artifact() = default;
    ~Artifact() override = default;

    template<typename TArtifact, typename Data> friend struct ArtifactData;

    std::string Name;
};

#define FOR_EACH_IMAGE_ARTIFACT(DO) \
    DO(IMAGE_GAUSSIAN, Gaussian) \
    DO(IMAGE_COMPOSITION, Composite)
//    DO(IMAGE_SALT_PEPPER, SaltPepper) \
//    DO(IMAGE_RING, Ring) \
//    DO(IMAGE_CUPPING, Cupping) \
//    DO(IMAGE_WIND_MILL, WindMill) \
//    DO(IMAGE_STAIR_STEP, StairStep) \
//    DO(IMAGE_STREAKING, Streaking) \


template<typename TArtifact, typename TData>
struct ArtifactData {
    QString Name;
    Artifact::Type Type = Artifact::Type::INVALID;
    Artifact::SubType SubType = Artifact::SubType::INVALID;

    static std::unique_ptr<TData> GetData(const TArtifact& artifact);

    static void SetData(TArtifact& artifact, const TData& data);

    static void SetData(TArtifact& artifact, const QVariant& variant);

protected:
    ArtifactData() = default;
    virtual ~ArtifactData() = default;

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
