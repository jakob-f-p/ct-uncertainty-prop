#pragma once

#include "../Artifacts/Artifact.h"

#include <vtkObject.h>

class BasicStructure;
class CombinedStructure;
class SimpleTransform;

template<typename Structure, typename Data> struct CtStructureData;
struct StructureArtifactList;

class CtStructure : public vtkObject {
    Q_GADGET

public:
    vtkTypeMacro(CtStructure, vtkObject)
    void PrintSelf(ostream& os, vtkIndent indent) override;
    vtkMTimeType GetMTime() override;

    void SetName(std::string name);

    virtual void SetTransform(const std::array<std::array<float, 3>, 3>& trs) = 0;

    CombinedStructure* GetParent() const;
    void SetParent(CombinedStructure* parent);

    enum SubType {
        BASIC,
        COMBINED
    };
    virtual SubType GetSubType() const = 0;
    bool IsBasic() const;
    static bool IsBasic(void* ctStructure);
    static BasicStructure* ToBasic(void* basicStructure);
    static BasicStructure* ToBasic(CtStructure* basicStructure);
    static CombinedStructure* ToCombined(void* combinedStructure);
    static CombinedStructure* ToCombined(CtStructure* combinedStructure);
    static CtStructure* FromVoid(void* ctStructure);

    virtual void DeepCopy(CtStructure* source, CombinedStructure* parent);

    struct Result {
        float FunctionValue = 0;
        float IntensityValue = 0;
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

    CtStructure(const CtStructure&) = delete;
    void operator=(const CtStructure&) = delete;

protected:
    CtStructure();
    ~CtStructure() override;

    virtual std::string GetViewName() const = 0;

    template<typename Structure, typename Data> friend struct CtStructureData;

    std::string Name;
    SimpleTransform* Transform;
    StructureArtifactList* StructureArtifacts;
    CombinedStructure* Parent;
};



template<typename Structure, typename Data>
struct CtStructureData {
    QString Name;
    QString ViewName;
    std::array<std::array<float, 3>, 3> Transform = {};

    static Data GetData(const Structure& structure);

    static QVariant GetQVariant(const Structure& structure);

    static void SetData(Structure& structure, const Data& data);

    static void SetData(Structure& structure, const QVariant& variant);

protected:
    CtStructureData() = default;
};



template<typename Ui, typename Data>
class CtStructureUi {
public:
    static QWidget* GetWidget();

    static Data GetWidgetData(QWidget* widget);

    static void SetWidgetData(QWidget* widget, const Data& data);

protected:
    static void AddCoordinatesRow(const QString& baseName, const QString& labelText,
                                  double minValue, double maxValue, double stepSize,
                                  QGridLayout* gridLayout, int gridLayoutRow,
                                  double defaultValue = 0.0);
    static QWidget* GetCoordinatesRow(const QString& baseName,
                                      double minValue, double maxValue, double stepSize);
    static QString GetAxisSpinBoxName(const QString& transformName, const QString& axisName);

    static const QStringList AxisNames;

private:
    static const QString NameEditObjectName;
    static const QStringList TransformNames;
};
