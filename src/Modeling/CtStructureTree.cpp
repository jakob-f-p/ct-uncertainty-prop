#include "CtStructureTree.h"

#include "BasicStructure.h"
#include "../Artifacts/PipelineList.h"
#include "../App.h"

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

void CtStructureTree::SetPipelineList(PipelineList& pipelineList) {
    Pipelines = &pipelineList;
}

void CtStructureTree::AddBasicStructure(BasicStructure& basicStructure, CombinedStructure* parent) {
    if (!parent) {  // add as root
        if (Root) {
            vtkErrorMacro("Another root is already present. Cannot add implicit Structure.");
            return;
        }

        Root = &basicStructure;
        Root->SetParent(nullptr);
        EmitEvent({ CtStructureTreeEventType::Add, *Root });

        Modified();
        return;
    }

    // add as child
    if (CtStructureExists(basicStructure)) {
        vtkErrorMacro("CT Structure already exists. Cannot add existing Structure.");
        return;
    }
    parent->AddCtStructure(basicStructure);
    EmitEvent({ CtStructureTreeEventType::Add, basicStructure });
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
        vtkErrorMacro("CT Structure already exists. Cannot combine existing Structure.");
        return;
    }

    combinedStructure.AddCtStructure(*Root);
    combinedStructure.AddCtStructure(basicStructure);
    EmitEvent({ CtStructureTreeEventType::Add, combinedStructure });
    EmitEvent({ CtStructureTreeEventType::Add, basicStructure });

    combinedStructure.SetParent(nullptr);
    Root->SetParent(&combinedStructure);
    basicStructure.SetParent(&combinedStructure);

    Root = &combinedStructure;

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

    auto* parent = dynamic_cast<CombinedStructure*>(structureToRefine.GetParent());
    combinedStructure->SetParent(parent);

    combinedStructure->AddCtStructure(structureToRefine);
    combinedStructure->AddCtStructure(*newStructure);
    EmitEvent({ CtStructureTreeEventType::Add, structureToRefine });
    EmitEvent({ CtStructureTreeEventType::Add, *newStructure });

    if (!parent)
        Root = combinedStructure;
    else
        parent->ReplaceChild(&structureToRefine, combinedStructure);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

void CtStructureTree::RemoveBasicStructure(BasicStructure& basicStructure) {
    if (!CtStructureExists(basicStructure)) {
        vtkErrorMacro("CT Structure to remove does not exist. Cannot remove non-existing Structure");
        return;
    }

    if (Root == &basicStructure) {
        EmitEvent({ CtStructureTreeEventType::Add, *Root });
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

    EmitEvent({ CtStructureTreeEventType::Add, *parent });
    CtStructure* newRoot = parent->RemoveBasicStructure(&basicStructure, grandParent);
    if (newRoot) {
        Root = newRoot;
        newRoot->SetParent(nullptr);
    }

    Modified();
}

void CtStructureTree::SetData(CtStructure* ctStructure, const QVariant& data) {
    if (ctStructure->IsBasic())
        BasicStructureData::SetData(*dynamic_cast<BasicStructure*>(ctStructure), data);
    else
        CombinedStructureData::SetData(*dynamic_cast<CombinedStructure*>(ctStructure), data);

    InvokeEvent(vtkCommand::ModifiedEvent);
}

CtStructure* CtStructureTree::GetRoot() const {
    return Root;
}

bool CtStructureTree::CtStructureExists(const CtStructure& ctStructure) {
    return Root && Root->CtStructureExists(&ctStructure);
}

void CtStructureTree::Iterate(const std::function<void(CtStructure&)>& f) const {
    if (!Root) return;

    Root->Iterate(f);
}

void CtStructureTree::EmitEvent(CtStructureTreeEvent event) {
    if (!Pipelines) {
        vtkWarningMacro("Pipeline must not be nullptr");
        return;
    }

    Pipelines->ProcessCtStructureTreeEvent(event);
}
