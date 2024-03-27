#pragma once

#include "../Artifacts/Artifact.h"

#include <QVariant>

#include <vtkObject.h>
#include <vtkImplicitFunction.h>

class CombinedStructure;
class SimpleTransform;

struct CtStructureDetails;
struct StructureArtifactList;

class CtStructure : public vtkObject {
    Q_GADGET

public:
    vtkTypeMacro(CtStructure, vtkObject)
    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    void SetName(std::string name);

    virtual void SetTransform(const std::array<std::array<float, 3>, 3>& trs) = 0;

    struct Result {
        float FunctionValue;
        float IntensityValue;
        std::map<Artifact::SubType, float> ArtifactValueMap;
    };
    virtual void EvaluateAtPosition(const double x[3], Result& result) = 0;

    struct ModelingResult {
        float FunctionValue;
        float Radiodensity;
        int BasicCtStructureId;
    };
    virtual const ModelingResult EvaluateImplicitModel(const double x[3]) const = 0;

    /**
     * Return f(x, y z) where f > 0 is outside of the surface, f = 0 is on the surface, and f < 0 is inside the surface.
     * Additionally, the distance to the surface is positively correlated with the function value at a given position.
     * Function takes the transform of the object into account.
     * @param x the input position vector consisting of x, y, and z coordinates
     * @return the function value f(x, y, z) at position x
     */
    virtual float FunctionValue(const double x[3]) const = 0;

    virtual bool CtStructureExists(const CtStructure* structure) = 0;

    CombinedStructure* GetParent() const;
    void SetParent(CombinedStructure* parent);

    virtual QVariant Data() const = 0;
    virtual void SetData(const QVariant& variant) = 0;

    enum SubType : unsigned int {
        BASIC,
        COMBINED
    };
    virtual SubType GetSubType() const = 0;
    bool IsBasicStructure() const;

    virtual void DeepCopy(CtStructure* source, CombinedStructure* parent);

    CtStructure(const CtStructure&) = delete;
    void operator=(const CtStructure&) = delete;

protected:
    CtStructure();
    ~CtStructure() override;

    virtual std::string GetViewName() const = 0;
    CtStructureDetails GetCtStructureDetails() const;
    void SetCtStructureDetails(const CtStructureDetails& ctStructureDetails);

    static void AddNameEditWidget(QLayout* layout);
    static void AddTransformEditWidget(QLayout* layout);
    static void SetEditWidgetData(QWidget* widget, const CtStructureDetails& ctStructureDetails);
    static CtStructureDetails GetEditWidgetData(QWidget* widget);

    friend class CtStructureEditDialog;

    std::string Name;
    SimpleTransform* Transform;
    StructureArtifactList* StructureArtifacts;
    CombinedStructure* Parent;    // always of type implicit structure combination

private:
    static void CreateTransformationEditGroup(const QString& transformName,
                                              double stepSize,
                                              QVBoxLayout* parentLayout);
    static QString GetSpinBoxName(const QString& transformName, const QString& axisName);

    static QString NameEditObjectName;
    static QStringList TransformNames;
    static QStringList AxisNames;
};

struct CtStructureDetails {
    QString Name;
    QString ViewName;
    std::array<std::array<float, 3>, 3> Transform;
};

