#include "CtDataCsgTree.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(CtDataCsgTree)

void CtDataCsgTree::PrintSelf(ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Root:\n";

    Root->PrintSelf(os, indent.GetNextIndent());
}

vtkMTimeType CtDataCsgTree::GetMTime() {
    vtkMTimeType thisMTime = vtkObject::GetMTime();
    vtkMTimeType nodeMTime = Root->GetMTime();

    return std::max(thisMTime, nodeMTime);
}

void CtDataCsgTree::AddImplicitCtStructure(ImplicitCtStructure& implicitCtStructure,
                                           ImplicitStructureCombination* parent) {
    if (!parent) {  // add as root
        if (Root) {
            vtkErrorMacro("Another root is already present. Cannot add implicit structure.");
            return;
        }

        Root = &implicitCtStructure;
        Root->Register(this);

        this->Modified();
        return;
    }

    // add as child
    if (CtStructureExists(implicitCtStructure)) {
        vtkErrorMacro("CT structure already exists. Cannot add existing structure.");
        return;
    }
    parent->AddCtStructure(implicitCtStructure);
}

void CtDataCsgTree::CombineWithImplicitCtStructure(ImplicitCtStructure& implicitCtStructure,
                                                   ImplicitStructureCombination::OperatorType operatorType) {
    if (!Root) {
        vtkErrorMacro("No root is present yet. Cannot combine.");
        return;
    }

    if (CtStructureExists(implicitCtStructure)) {
        vtkErrorMacro("CT structure already exists. Cannot combine existing structure.");
        return;
    }

    CtStructure* previousRoot = Root;

    ImplicitStructureCombination* newRoot = ImplicitStructureCombination::New();
    newRoot->SetOperatorType(operatorType);
    newRoot->AddCtStructure(*previousRoot);
    newRoot->AddCtStructure(implicitCtStructure);

    previousRoot->UnRegister(this);

    Root = newRoot;
}

void CtDataCsgTree::RemoveImplicitCtStructure(ImplicitCtStructure& implicitCtStructure) {
    if (!CtStructureExists(implicitCtStructure)) {
        vtkErrorMacro("CT structure to remove does not exist. Cannot remove non-existing structure");
        return;
    }

    if (Root == &implicitCtStructure) {
        Root->UnRegister(this);
        Root = nullptr;
        return;
    }

    ImplicitStructureCombination* root = ImplicitStructureCombination::SafeDownCast(Root);
    ImplicitStructureCombination* parent = root->FindParentOfCtStructure(implicitCtStructure);

    if (!parent) {
        vtkErrorMacro("Parent cannot be null here");
        return;
    }

    ImplicitStructureCombination* grandParent = root->FindParentOfCtStructure(*parent);

    parent->RemoveImplicitCtStructure(&implicitCtStructure, grandParent);
}

CtDataCsgTree::CtDataCsgTree() {
    Root = nullptr;
}

CtDataCsgTree::~CtDataCsgTree() {
    if (Root) {
        Root->Delete();
    }
}

bool CtDataCsgTree::CtStructureExists(const CtStructure& ctStructure) {
    return Root && Root->CtStructureExists(&ctStructure);
}
