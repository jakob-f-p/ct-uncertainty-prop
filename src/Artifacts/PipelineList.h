#pragma once

#include <vtkObject.h>

class Pipeline;

class PipelineList : public vtkObject {
public:
    static PipelineList* New();
    vtkTypeMacro(PipelineList, vtkObject)

    void PrintSelf(std::ostream& os, vtkIndent indent) override;

    bool IsEmpty() const;

    int GetSize() const;

    Pipeline* Get(int idx) const;

    void AddPipeline(Pipeline* pipeline);

    void RemovePipeline(Pipeline* pipeline);

    int NumberOfPipelines() const;

    PipelineList(const PipelineList&) = delete;
    void operator=(const PipelineList&) = delete;

protected:
    PipelineList() = default;
    ~PipelineList() override = default;

    std::vector<Pipeline*> Pipelines;
};
