#include "CtStructureTree.h"

#include "BasicStructure.h"

#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(CtStructureTree)

void CtStructureTree::PrintSelf(ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Root:\n";

    Root->PrintSelf(os, indent.GetNextIndent());
}

vtkMTimeType CtStructureTree::GetMTime() {
    vtkMTimeType thisMTime = vtkObject::GetMTime();
    vtkMTimeType nodeMTime = Root ? Root->GetMTime() : 0;

    return std::max(thisMTime, nodeMTime);
}

void CtStructureTree::AddBasicStructure(BasicStructure& basicStructure, CombinedStructure* parent) {
    if (!parent) {  // add as root
        if (Root) {
            vtkErrorMacro("Another root is already present. Cannot add implicit structure.");
            return;
        }

        Root = &basicStructure;
        Root->SetParent(nullptr);
        Root->Register(this);

        this->Modified();
        return;
    }

    // add as child
    if (CtStructureExists(basicStructure)) {
        vtkErrorMacro("CT structure already exists. Cannot add existing structure.");
        return;
    }
    parent->AddCtStructure(basicStructure);
}

void CtStructureTree::AddBasicStructure(const BasicStructureData& basicStructureData, CombinedStructure *parent) {
    vtkNew<BasicStructure> basicStructure;
    BasicStructureData::SetData(*basicStructure, basicStructureData);
    AddBasicStructure(*basicStructure, parent);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtStructureTree::CombineWithBasicStructure(BasicStructure& basicStructure,
                                                CombinedStructure::OperatorType operatorType) {
    if (!Root) {
        vtkErrorMacro("No root is present yet. Cannot combine.");
        return;
    }

    if (CtStructureExists(basicStructure)) {
        vtkErrorMacro("CT structure already exists. Cannot combine existing structure.");
        return;
    }

    CtStructure* previousRoot = Root;

    CombinedStructure* newRoot = CombinedStructure::New();
    newRoot->SetOperatorType(operatorType);
    newRoot->AddCtStructure(*previousRoot);
    newRoot->AddCtStructure(basicStructure);

    previousRoot->UnRegister(this);

    newRoot->SetParent(nullptr);
    previousRoot->SetParent(newRoot);
    basicStructure.SetParent(newRoot);

    Root = newRoot;

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtStructureTree::CombineWithBasicStructure(BasicStructureData& basicStructureData) {
    vtkNew<BasicStructure> basicStructure;
    BasicStructureData::SetData(*basicStructure, basicStructureData);
    CombineWithBasicStructure(*basicStructure, CombinedStructure::OperatorType::UNION);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtStructureTree::RefineWithBasicStructure(const BasicStructureData& newStructureData,
                                               BasicStructure& structureToRefine,
                                               CombinedStructure::OperatorType operatorType) {
    if (!Root) {
        vtkErrorMacro("No root is present yet. Cannot refine.");
        return;
    }

    vtkNew<BasicStructure> newStructure;
    BasicStructureData::SetData(*newStructure, newStructureData);

    auto* parent = dynamic_cast<CombinedStructure*>(structureToRefine.GetParent());

    CombinedStructure* combination = CombinedStructure::New();
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

void CtStructureTree::RemoveBasicStructure(BasicStructure& basicStructure) {
    if (!CtStructureExists(basicStructure)) {
        vtkErrorMacro("CT structure to remove does not exist. Cannot remove non-existing structure");
        return;
    }

    if (Root == &basicStructure) {
        Root->UnRegister(this);
        Root = nullptr;
        InvokeEvent(vtkCommand::ModifiedEvent);
        return;
    }

    auto* parent = dynamic_cast<CombinedStructure*>(basicStructure.GetParent());

    if (!parent) {
        vtkErrorMacro("Parent cannot be null here");
        return;
    }

    auto* grandParent = dynamic_cast<CombinedStructure*>(parent->GetParent());

    CtStructure* newRoot = parent->RemoveBasicStructure(&basicStructure, grandParent);
    if (newRoot) {
        newRoot->Register(this);
        Root->Delete();
        Root = newRoot;
        newRoot->SetParent(nullptr);
    }

    Modified();
}

void CtStructureTree::SetData(CtStructure* ctStructure, const QVariant& data) {
    if (ctStructure->IsBasic()) {
        BasicStructureData::SetData(*dynamic_cast<BasicStructure*>(ctStructure), data);
    } else {
        CombinedStructureData::SetData(*dynamic_cast<CombinedStructure*>(ctStructure), data);
    }

    InvokeEvent(vtkCommand::ModifiedEvent);
}

CtStructureTree::CtStructureTree() {
    Root = nullptr;
}

CtStructureTree::~CtStructureTree() {
    if (Root) {
        Root->Delete();
    }
}

CtStructure* CtStructureTree::GetRoot() const {
    return Root;
}

bool CtStructureTree::CtStructureExists(const CtStructure& ctStructure) {
    return Root && Root->CtStructureExists(&ctStructure);
}

void CtStructureTree::DeepCopy(CtStructureTree* source) {
    if (!source->Root) {
        Root = nullptr;
        return;
    }

    if (Root)
        Root->Delete();

    if (source->Root->IsBasic())
        Root = BasicStructure::New();
    else
        Root = CombinedStructure::New();

    Root->DeepCopy(source->Root, nullptr);
}
