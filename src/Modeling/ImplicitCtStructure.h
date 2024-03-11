#pragma once

#include <mutex>

#include <vtkImplicitFunction.h>

#include "CT.h"
#include "../Artifacts/StructureArtifactList.h"


/**
 * @class ImplicitCtStructure
 * @brief class representing implicit CT structures with structure Artifacts
 *
 * This class is used in ImplicitCsgTree as an implicit source of data.
 */
class ImplicitCtStructure : public vtkObject {
public:
    static ImplicitCtStructure* New();
    vtkTypeMacro(ImplicitCtStructure, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    /**
     * Set/Get the implicit function separating the function domain into position inside, on, and outside of the
     * surface. The domain on and inside the surface (f(x,y,z) <= 0) constitutes the structure.
     */
    vtkSetObjectMacro(ImplicitFunction, vtkImplicitFunction);
    vtkGetObjectMacro(ImplicitFunction, vtkImplicitFunction);

    /**
     * Set the type of tissue.
     */
    void SetTissueType(CT::TissueOrMaterialType tissueType);

    /**
     * Return f(x, y z) where f > 0 is outside of the surface, f = 0 is on the surface, and f < 0 is inside the surface.
     * Additionally, the distance to the surface is positively correlated with the function value at a given position.
     * @param x the input position vector consisting of x, y, and z coordinates
     * @return the function value f(x, y, z) at position x
     */
    double FunctionValue(const double x[3]);

    ImplicitCtStructure(const ImplicitCtStructure&) = delete;
    void operator=(const ImplicitCtStructure&) = delete;

protected:
    ImplicitCtStructure();
    ~ImplicitCtStructure() override;

    vtkImplicitFunction* ImplicitFunction;
    CT::TissueOrMaterialType Tissue;
    StructureArtifactList* StructureArtifacts;
};