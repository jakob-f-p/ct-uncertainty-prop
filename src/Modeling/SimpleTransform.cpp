#include "SimpleTransform.h"

#include <QVariant>

#include <vtkObjectFactory.h>

vtkStandardNewMacro(SimpleTransform)

void SimpleTransform::PrintSelf(ostream& os, vtkIndent indent) {
    os << indent << "Translation: [" << TranslationValues[0] << ", " << TranslationValues[1] << ", " << TranslationValues[2] << "]\n";
    os << indent << "Rotation: [" << RotationAngles[0] << ", " << RotationAngles[1] << ", " << RotationAngles[2] << "]\n";
    os << indent << "Scale: [" << ScaleFactors[0] << ", " << ScaleFactors[1] << ", " << ScaleFactors[2] << "]\n";
}

SimpleTransform::SimpleTransform() :
    TranslationValues{ 0.0f, 0.0f, 0.0f },
    RotationAngles   { 0.0f, 0.0f, 0.0f },
    ScaleFactors     { 1.0f, 1.0f, 1.0f } {
}

std::array<std::array<float, 3>, 3> SimpleTransform::GetTranslationRotationScaling() {
    std::array<std::array<float, 3>, 3> trs {};
    trs[0] = TranslationValues;
    trs[1] = RotationAngles;
    trs[2] = ScaleFactors;

    return trs;
}

void SimpleTransform::SetTranslationRotationScaling(const std::array<std::array<float, 3>, 3>& trs) {
    TranslationValues = trs[0];
    RotationAngles =    trs[1];
    ScaleFactors =      trs[2];

    Modified();
}

void SimpleTransform::InternalDeepCopy(vtkAbstractTransform* copy) {
    auto* transform = dynamic_cast<SimpleTransform*>(copy);

    transform->SetTranslationRotationScaling(GetTranslationRotationScaling());

    this->Superclass::DeepCopy(copy);
}

//------------------------------------------------------------------------------
void SimpleTransform::InternalUpdate() {
    this->Superclass::Identity();

    Translate(TranslationValues.data());
    RotateX(RotationAngles[0]);
    RotateY(RotationAngles[1]);
    RotateZ(RotationAngles[2]);
    Scale(ScaleFactors.data());
    Inverse();

    this->Superclass::InternalUpdate();
}
