#pragma once

#include <QList>

#include <vtkTransform.h>

#include <array>

class SimpleTransform : public vtkTransform {
public:
    static SimpleTransform* New();
    vtkTypeMacro(SimpleTransform, vtkTransform);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    QVariant GetTranslationRotationScaling();
    void SetTranslationRotationScaling(const QVariant& trs);

    SimpleTransform(const SimpleTransform&) = delete;
    void operator=(const SimpleTransform&) = delete;

protected:
    SimpleTransform();
    ~SimpleTransform() override = default;

    void InternalDeepCopy(vtkAbstractTransform* copy) override;

    void InternalUpdate() override;

    std::array<float, 3> TranslationValues;
    std::array<float, 3> RotationAngles;
    std::array<float, 3> ScaleFactors;
};
