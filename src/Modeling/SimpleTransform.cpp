#include "SimpleTransform.h"

#include <QVariant>

#include <vtkObjectFactory.h>

vtkStandardNewMacro(SimpleTransform)

void SimpleTransform::PrintSelf(ostream& os, vtkIndent indent) {
    vtkTransform::PrintSelf(os, indent);

    os << indent << "Translation: [" << TranslationValues[0] << ", " << TranslationValues[1] << ", " << TranslationValues[2] << "]\n";
    os << indent << "Rotation: [" << RotationAngles[0] << ", " << RotationAngles[1] << ", " << RotationAngles[2] << "]\n";
    os << indent << "Scale: [" << ScaleFactors[0] << ", " << ScaleFactors[1] << ", " << ScaleFactors[2] << "]\n";
}

SimpleTransform::SimpleTransform() :
    TranslationValues{ 0.0f, 0.0f, 0.0f },
    RotationAngles   { 0.0f, 0.0f, 0.0f },
    ScaleFactors     { 1.0f, 1.0f, 1.0f } {
}

QVariant SimpleTransform::GetTranslationRotationScaling() {
    QList<QVariant> list;

    list.push_back(QList<QVariant>(TranslationValues.begin(), TranslationValues.end()));
    list.push_back(QList<QVariant>(RotationAngles.begin(), RotationAngles.end()));
    list.push_back(QList<QVariant>(ScaleFactors.begin(), ScaleFactors.end()));

    return list;
}

void SimpleTransform::SetTranslationRotationScaling(const QVariant& trs) {
    QVariantList vectorList = trs.toList();

    QVariantList t = vectorList.at(0).toList();
    QVariantList r = vectorList.at(1).toList();
    QVariantList s = vectorList.at(2).toList();

    TranslationValues = { t[0].toFloat(), t[1].toFloat(), t[2].toFloat() };
    RotationAngles =    { r[0].toFloat(), r[1].toFloat(), r[2].toFloat() };
    ScaleFactors =      { s[0].toFloat(), s[2].toFloat(), s[2].toFloat() };
}

void SimpleTransform::InternalDeepCopy(vtkAbstractTransform* copy) {
    auto* transform = static_cast<SimpleTransform*>(copy);

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

    this->Superclass::InternalUpdate();
}
