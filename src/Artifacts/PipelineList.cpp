#include "PipelineList.h"

#include "Pipeline.h"
#include "../Modeling/CtStructureTree.h"

#include <QMessageLogger>

#include <vtkObjectFactory.h>

vtkStandardNewMacro(PipelineList)

void PipelineList::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

bool PipelineList::IsEmpty() const {
    return Pipelines.empty();
}

int PipelineList::GetSize() const {
    return static_cast<int>(Pipelines.size());
}

Pipeline* PipelineList::Get(int idx) const {
    return Pipelines.at(idx);
}

void PipelineList::AddPipeline(Pipeline* pipeline) {
    if (!pipeline) {
        qWarning("Given pipeline was nullptr");
        return;
    }

    Pipelines.emplace_back(pipeline);
}

void PipelineList::RemovePipeline(Pipeline* pipeline) {
    auto removeIt = std::find(Pipelines.begin(), Pipelines.end(), pipeline);
    if (removeIt == Pipelines.end()) {
        vtkWarningMacro("Given pipeline could not be removed because it was not present");
        return;
    }

    Pipelines.erase(removeIt);
}

int PipelineList::NumberOfPipelines() const {
    return static_cast<int>(Pipelines.size());
}

void PipelineList::ProcessCtStructureTreeEvent(CtStructureTreeEvent event) {
    for (auto& pipeline: Pipelines) {
        pipeline->ProcessCtStructureTreeEvent(event);
        Modified();
    }
}
