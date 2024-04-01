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

void CtStructureTree::CombineWithBasicStructure(BasicStructure& basicStructure, CombinedStructure& combinedStructure) {
    if (!Root) {
        vtkErrorMacro("No root is present yet. Cannot combine.");
        return;
    }

    if (CtStructureExists(basicStructure) || CtStructureExists(combinedStructure)) {
        vtkErrorMacro("CT structure already exists. Cannot combine existing structure.");
        return;
    }

    CtStructure* previousRoot = Root;

    combinedStructure.AddCtStructure(*previousRoot);
    combinedStructure.AddCtStructure(basicStructure);

    combinedStructure.SetParent(nullptr);
    previousRoot->SetParent(&combinedStructure);
    basicStructure.SetParent(&combinedStructure);

    Root = &combinedStructure;

    previousRoot->UnRegister(this);
    combinedStructure.Register(this);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtStructureTree::CombineWithBasicStructure(const BasicStructureData& basicStructureData,
                                                const CombinedStructureData& combinedStructureData) {
    vtkNew<BasicStructure> basicStructure;
    BasicStructureData::SetData(*basicStructure, basicStructureData);

    vtkNew<CombinedStructure> combinedStructure;
    CombinedStructureData::SetData(*combinedStructure, combinedStructureData);

    CombineWithBasicStructure(*basicStructure, *combinedStructure);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtStructureTree::RefineWithBasicStructure(const BasicStructureData& newStructureData,
                                               const CombinedStructureData& combinedStructureData,
                                               BasicStructure& structureToRefine) {
    if (!Root) {
        vtkErrorMacro("No root is present yet. Cannot refine.");
        return;
    }

    vtkNew<BasicStructure> newStructure;
    BasicStructureData::SetData(*newStructure, newStructureData);

    vtkNew<CombinedStructure> combinedStructure;
    CombinedStructureData::SetData(*combinedStructure, combinedStructureData);
    combinedStructure->Register(this);

    auto* parent = dynamic_cast<CombinedStructure*>(structureToRefine.GetParent());
    combinedStructure->SetParent(parent);

    combinedStructure->AddCtStructure(structureToRefine);
    combinedStructure->AddCtStructure(*newStructure);

    if (!parent) {
        structureToRefine.UnRegister(this);
        Root = combinedStructure;
        Root->Register(this);
        return;
    }
    parent->ReplaceChild(&structureToRefine, combinedStructure);

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
