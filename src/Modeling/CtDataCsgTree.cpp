#include "CtDataCsgTree.h"

#include "ImplicitCtStructure.h"

#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(CtDataCsgTree)

void CtDataCsgTree::PrintSelf(ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Root:\n";

    Root->PrintSelf(os, indent.GetNextIndent());
}

vtkMTimeType CtDataCsgTree::GetMTime() {
    vtkMTimeType thisMTime = vtkObject::GetMTime();
    vtkMTimeType nodeMTime = Root ? Root->GetMTime() : 0;

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
        Root->SetParent(nullptr);
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

void CtDataCsgTree::AddImplicitCtStructure(const ImplicitCtStructureDetails &implicitCtStructureDetails,
                                           ImplicitStructureCombination *parent) {
    vtkNew<ImplicitCtStructure> implicitCtStructure;
    implicitCtStructure->SetData(implicitCtStructureDetails);
    AddImplicitCtStructure(*implicitCtStructure, parent);

    InvokeEvent(vtkCommand::ModifiedEvent);
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

    newRoot->SetParent(nullptr);
    previousRoot->SetParent(newRoot);
    implicitCtStructure.SetParent(newRoot);

    Root = newRoot;

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtDataCsgTree::CombineWithImplicitCtStructure(ImplicitCtStructureDetails &implicitCtStructureDetails) {
    vtkNew<ImplicitCtStructure> implicitCtStructure;
    implicitCtStructure->SetData(implicitCtStructureDetails);
    CombineWithImplicitCtStructure(*implicitCtStructure, ImplicitStructureCombination::OperatorType::UNION);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtDataCsgTree::RefineWithImplicitStructure(const ImplicitCtStructureDetails& newStructureDetails,
                                                ImplicitCtStructure& structureToRefine,
                                                ImplicitStructureCombination::OperatorType operatorType) {
    if (!Root) {
        vtkErrorMacro("No root is present yet. Cannot refine.");
        return;
    }

    vtkNew<ImplicitCtStructure> newStructure;
    newStructure->SetData(newStructureDetails);

    auto* parent = dynamic_cast<ImplicitStructureCombination*>(structureToRefine.GetParent());

    ImplicitStructureCombination* combination = ImplicitStructureCombination::New();
    combination->SetOperatorType(operatorType);
    combination->SetParent(parent);

    combination->AddCtStructure(structureToRefine);
    combination->AddCtStructure(*newStructure);

    if (!parent) {
        structureToRefine.UnRegister(this);
        Root = combination;
        Root->Register(this);
        return;
    }
    parent->ReplaceChild(&structureToRefine, combination);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtDataCsgTree::RemoveImplicitCtStructure(ImplicitCtStructure& implicitCtStructure) {
    if (!CtStructureExists(implicitCtStructure)) {
        vtkErrorMacro("CT structure to remove does not exist. Cannot remove non-existing structure");
        return;
    }

    if (Root == &implicitCtStructure) {
        Root->UnRegister(this);
        Root = nullptr;
        InvokeEvent(vtkCommand::ModifiedEvent);
        return;
    }

    auto* parent = dynamic_cast<ImplicitStructureCombination*>(implicitCtStructure.GetParent());

    if (!parent) {
        vtkErrorMacro("Parent cannot be null here");
        return;
    }

    auto* grandParent = dynamic_cast<ImplicitStructureCombination*>(parent->GetParent());

    CtStructure* newRoot = parent->RemoveImplicitCtStructure(&implicitCtStructure, grandParent);
    if (newRoot) {
        newRoot->Register(this);
        Root->Delete();
        Root = newRoot;
        newRoot->SetParent(nullptr);
    }

    Modified();
}

void CtDataCsgTree::SetData(CtStructure* ctStructure, const QVariant& data) {
    ctStructure->SetData(data);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

CtDataCsgTree::CtDataCsgTree() {
    Root = nullptr;
}

CtDataCsgTree::~CtDataCsgTree() {
    if (Root) {
        Root->Delete();
    }
}

CtStructure* CtDataCsgTree::GetRoot() const {
    return Root;
}

bool CtDataCsgTree::CtStructureExists(const CtStructure& ctStructure) {
    return Root && Root->CtStructureExists(&ctStructure);
}

void CtDataCsgTree::DeepCopy(CtDataCsgTree* source) {
    if (!source->Root) {
        Root = nullptr;
        return;
    }

    if (Root) {
        Root->Delete();
    }

    if (source->Root->IsImplicitCtStructure()) {
        Root = ImplicitCtStructure::New();
    } else {
        Root = ImplicitStructureCombination::New();
    }

    Root->DeepCopy(source->Root, nullptr);
}
