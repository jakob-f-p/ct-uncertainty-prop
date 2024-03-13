#include "CtStructure.h"

void CtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
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
}
