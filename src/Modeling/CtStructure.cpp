#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include "ImplicitCtStructure.h"
#include "CtStructure.h"

#include <utility>

void CtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Name: " << Name << "\n";
    os << indent << "View Name: " << GetViewName() << "\n";
    os << indent << "Transform: " << "\n";
    Transform->PrintSelf(os, indent.GetNextIndent());
    os << indent << "Parent: " << Parent << std::endl;
}

vtkMTimeType CtStructure::GetMTime() {
    return std::max({ Superclass::GetMTime(), Transform->GetMTime() });
}

void CtStructure::SetName(std::string name) {
    Name = std::move(name);

    this->Modified();
}

std::string CtStructure::GetName() const {
    return Name;
}

const SimpleTransform* CtStructure::GetTransform() const {
    return Transform;
}

CtStructure* CtStructure::GetParent() const {
    return Parent;
}

void CtStructure::SetParent(CtStructure* parent) {
    Parent = parent;
}

int CtStructure::ChildIndex() const {
    if (!Parent) {
        return 0;
    }

    auto* childrenOfParent = Parent->GetChildren();
    if (!childrenOfParent) {
        qWarning("Children cannot be nullptr");
        return -1;
    }

    auto searchIt = std::find(childrenOfParent->begin(), childrenOfParent->end(), this);

    if(searchIt == childrenOfParent->end()) {
        qWarning("Structures is not child of parent");
    }

    return static_cast<int>(std::distance(childrenOfParent->begin(), searchIt));
}

void CtStructure::DeepCopy(CtStructure* source, CtStructure* parent) {
    Name = source->Name;
    Transform->DeepCopy(source->Transform);
    Parent = parent;
}

CtStructure::CtStructure() {
    Parent = nullptr;
    Transform = SimpleTransform::New();
}

CtStructure::~CtStructure() {
    Transform->Delete();
}

CtStructureDetails CtStructure::GetCtStructureDetails() const {
    return {
        QString::fromStdString(Name),
        QString::fromStdString(GetViewName()),
        Transform->GetTranslationRotationScaling()
    };
}

void CtStructure::SetCtStructureDetails(const CtStructureDetails &ctStructureDetails) {
    SetName(ctStructureDetails.Name.toStdString());
    SetTransform(ctStructureDetails.Transform);
}
