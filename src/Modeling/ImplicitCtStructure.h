#pragma once

#include "CT.h"
#include "CtStructure.h"

#include "../Artifacts/StructureArtifactList.h"

#include <vtkImplicitFunction.h>


/**
 * @class ImplicitCtStructure
 * @brief class representing implicit CT structures with structure Artifacts
 *
 * This class is used in ImplicitCsgTree as an implicit source of data.
 */
class ImplicitCtStructure : public CtStructure {
public:
    static ImplicitCtStructure* New();
    vtkTypeMacro(ImplicitCtStructure, CtStructure);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    vtkMTimeType GetMTime() override;

    /**
     * Set/Get the implicit function separating the function domain into position inside, on, and outside of the
     * surface. The domain on and inside the surface (f(x,y,z) <= 0) constitutes the structure.
     */
    vtkSetObjectMacro(ImplicitFunction, vtkImplicitFunction);
    vtkGetObjectMacro(ImplicitFunction, vtkImplicitFunction);

    void SetTransform(vtkTransform* transform) override;
    vtkTransform* GetTransform() override;

    /**
     * Set the type of tissue.
     */
    void SetTissueType(CT::TissueOrMaterialType tissueType);

    void EvaluateAtPosition(const double x[3], Result& result) override;

    float FunctionValue(const double x[3]) override;

    bool CtStructureExists(const CtStructure* structure) override;

    int ChildCount() const override;

    int ColumnCount() const override;

    const std::vector<CtStructure*>& GetChildren() const override;

    const CtStructure* ChildAt(int idx) const override;

    QVariant Data(int idx) const override;

    ImplicitCtStructure(const ImplicitCtStructure&) = delete;
    void operator=(const ImplicitCtStructure&) = delete;

protected:
    ImplicitCtStructure();
    ~ImplicitCtStructure() override;

    vtkImplicitFunction* ImplicitFunction;
    CT::TissueOrMaterialType Tissue;
    StructureArtifactList* StructureArtifacts;
};