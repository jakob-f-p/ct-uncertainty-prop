#pragma once

#include "SimpleTransform.h"
#include "../Artifacts/StructureArtifactList.h"

#include <QVariant>

#include <vtkObject.h>
#include <vtkImplicitFunction.h>


struct CtStructureDetails {
    QString Name;
    QString ViewName;
    std::array<std::array<float, 3>, 3> Transform;
};

class CtStructure : public vtkObject {
    Q_GADGET
public:
    vtkTypeMacro(CtStructure, vtkObject)
    void PrintSelf(ostream& os, vtkIndent indent) override;
    void SetName(std::string name);

    std::string GetName() const;

    virtual void SetTransform(const std::array<std::array<float, 3>, 3>& trs) = 0;
    const SimpleTransform* GetTransform() const;

    struct Result {
        float FunctionValue;
        float IntensityValue;
        std::map<Artifact::SubType, float> ArtifactValueMap;
    };
    virtual void EvaluateAtPosition(const double x[3], Result& result) = 0;

    /**
     * Return f(x, y z) where f > 0 is outside of the surface, f = 0 is on the surface, and f < 0 is inside the surface.
     * Additionally, the distance to the surface is positively correlated with the function value at a given position.
     * Function takes the transform of the object into account.
     * @param x the input position vector consisting of x, y, and z coordinates
     * @return the function value f(x, y, z) at position x
     */
    virtual float FunctionValue(const double x[3]) = 0;

    virtual bool CtStructureExists(const CtStructure* structure) = 0;
    virtual int ChildCount() const = 0;

    static int ColumnCount();

    CtStructure* GetParent() const;

    void SetParent(CtStructure* parent);

    int ChildIndex() const;

    virtual const std::vector<CtStructure*>* GetChildren() const = 0;

    virtual const CtStructure* ChildAt(int idx) const = 0;

    virtual QVariant Data() const = 0;

    virtual void SetData(const QVariant& variant) = 0;

    virtual bool IsImplicitCtStructure() const = 0;

    CtStructure(const CtStructure&) = delete;
    void operator=(const CtStructure&) = delete;

protected:
    CtStructure();
    ~CtStructure() override;

    virtual std::string GetViewName() const = 0;
    CtStructureDetails GetCtStructureDetails() const;
    void SetCtStructureDetails(const CtStructureDetails& ctStructureDetails);

    std::string Name;
    SimpleTransform* Transform;
    CtStructure* Parent;    // always of type implicit structure combination
};

template<class EnumType>
struct EnumString {
    QString Name;
    EnumType EnumValue = {};
};

#define GET_ENUM_VALUES(EnumType)                                   \
static std::vector<EnumString<EnumType>> Get##EnumType##Values() {  \
    auto metaEnum = QMetaEnum::fromType<EnumType>();                \
    std::vector<EnumString<EnumType>> values(metaEnum.keyCount());  \
    EnumType enumValue;                                             \
    for (int i = 0; i < values.size(); i++) {                       \
        enumValue = static_cast<EnumType>(metaEnum.value(i));       \
        values[i] = { EnumType##ToString(enumValue).c_str(), enumValue };   \
    }                                                               \
    return values;                                                  \
}                                                                   \

