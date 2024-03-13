#include "CtStructure.h"

#include <utility>

void CtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

void CtStructure::SetName(std::string name) {
    Name = std::move(name);
}
std::string CtStructure::GetName() const {
    return Name;
}

const SimpleTransform* CtStructure::GetTransform() const {
    return Transform;
}

QVariant CtStructure::GetTransformQVariant() const {
    return Transform->GetTranslationRotationScaling();
}

int CtStructure::ColumnCount() {
    return Column::NUMBER_OF_COLUMNS;
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

    auto& childrenOfParent = Parent->GetChildren();
    auto searchIt = std::find(childrenOfParent.begin(), childrenOfParent.end(), this);

    assert(searchIt != childrenOfParent.end());

    return static_cast<int>(std::distance(childrenOfParent.begin(), searchIt));
}

CtStructure::CtStructure() {
    Parent = nullptr;
    Transform = SimpleTransform::New();
}

CtStructure::~CtStructure() {
    Transform->Delete();
}
