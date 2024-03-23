#include "PipelineList.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(PipelineList)

void PipelineList::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

bool PipelineList::IsEmpty() const {
    return Pipelines.empty();
}

int PipelineList::GetSize() const {
    return Pipelines.size();
}

Pipeline* PipelineList::Get(int idx) const {
    return Pipelines.at(idx);
}

void PipelineList::AddPipeline(Pipeline* pipeline) {
    if (!pipeline) {
        qWarning("Given pipeline was nullptr");
        return;
    }

    pipeline->Register(this);

    Pipelines.push_back(pipeline);
}

void PipelineList::RemovePipeline(Pipeline* pipeline) {
    if (auto search = std::find(Pipelines.begin(), Pipelines.end(), pipeline);
            search == Pipelines.end()) {
        vtkWarningMacro("Given pipeline could not be removed because it was not present");
        return;
    }

    auto pastLastIt = std::remove(Pipelines.begin(), Pipelines.end(), pipeline);
    Pipelines.erase(pastLastIt);

    pipeline->Delete();
}

int PipelineList::NumberOfPipelines() {
    return static_cast<int>(Pipelines.size());
}
