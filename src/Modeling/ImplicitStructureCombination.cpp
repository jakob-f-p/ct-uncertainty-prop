#include "CT.h"
#include "ImplicitStructureCombination.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(ImplicitStructureCombination);

void ImplicitStructureCombination::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);

    os << indent << "Operator Type: " << GetOperatorTypeName() << "\n";

    os << indent << "Ct Structures:\n";
    auto nextIndent = indent.GetNextIndent();
    for (const auto& ctStructure: CtStructures) {
        ctStructure->PrintSelf(os, nextIndent);
    }
}

vtkMTimeType ImplicitStructureCombination::GetMTime() {
    vtkMTimeType thisMTime = this->CtStructure::GetMTime();
    auto maxMTimeStructureIt = std::max_element(CtStructures.begin(),
                                                      CtStructures.end(),
                                                      [](CtStructure* a, CtStructure* b) {
        return a && b && a->GetMTime() < b->GetMTime();
    });

    return maxMTimeStructureIt == CtStructures.end()
            ? thisMTime
            : std::max(thisMTime, (*maxMTimeStructureIt)->GetMTime());
}


void ImplicitStructureCombination::Delete() {
    for (const auto& structure : CtStructures) {
        structure->Delete();
    }

    Superclass::Delete();
}

void ImplicitStructureCombination::AddCtStructure(CtStructure& ctStructure) {
    if (auto search = std::find(CtStructures.begin(), CtStructures.end(), &ctStructure);
        search != CtStructures.end()) {
        vtkWarningMacro("Trying to add implicit CT structure which is already present");
    }

    CtStructures.push_back(&ctStructure);
    ctStructure.Register(this);

    this->Modified();
}

CtStructure* ImplicitStructureCombination::RemoveImplicitCtStructure(ImplicitCtStructure* implicitStructure,
                                                                     ImplicitStructureCombination* grandParent) {
    if (auto search = std::find(CtStructures.begin(), CtStructures.end(), implicitStructure);
        search == CtStructures.end()) {
        vtkWarningMacro("Given structure could not be removed because it was not present");
        return nullptr;
    }

    auto pastLastIt = std::remove(CtStructures.begin(), CtStructures.end(), implicitStructure);
    CtStructures.erase(pastLastIt);

    implicitStructure->Delete();


    if (CtStructures.size() == 1) {
        CtStructure* remainingStructure = CtStructures[0];

        if (!grandParent) {     // this node is root
            return remainingStructure;
        }

        grandParent->ReplaceConnection(this, remainingStructure);
    }

    return nullptr;
}

ImplicitStructureCombination::ImplicitStructureCombination() {
    this->OpType = UNION;
    this->Transform = nullptr;
}

ImplicitStructureCombination::~ImplicitStructureCombination() {
    if (this->Transform) {
        this->Transform->Delete();
    }

    for (const auto& structure : CtStructures) {
        structure->UnRegister(this);
    }
}

void ImplicitStructureCombination::SetOperatorType(ImplicitStructureCombination::OperatorType operatorType) {
    this->OpType = operatorType;
}

ImplicitStructureCombination::OperatorType ImplicitStructureCombination::GetOperatorType() {
    return this->OpType;
}

const char* ImplicitStructureCombination::GetOperatorTypeName() {
    switch (OpType) {
        case UNION:        return "Union";
        case INTERSECTION: return "Intersection";
        case DIFFERENCE:   return "Difference";
    }

    return nullptr;
}

void ImplicitStructureCombination::SetTransform(vtkAbstractTransform* transform) {
    vtkSetObjectBodyMacro(Transform, vtkAbstractTransform, transform);
}

vtkAbstractTransform* ImplicitStructureCombination::GetTransform() {
    return Transform;
}

void ImplicitStructureCombination::EvaluateAtPosition(const double x[3], CtStructure::Result& result) {
    if (CtStructures.empty()) {
        vtkErrorMacro("CtStructureList is empty. Cannot evaluate");
        return;
    }

    vtkWarningMacro("TODO: Implement correct artifact evaluation (add artifact values that ignore when they are covered by another structure)");

    auto results = std::vector<Result>(CtStructures.size());
    for (int i = 0; i < CtStructures.size(); ++i) {
        CtStructures[i]->EvaluateAtPosition(x, results[i]);
    }

    Result res;
    switch (OpType) {
        case UNION: {
            res = std::move(*std::min_element(results.begin(),
                                              results.end(),
                                              [](Result& a, Result& b) {
                return a.FunctionValue < b.FunctionValue;
            }));
            break;
        }

        case INTERSECTION: {
            res = std::move(*std::max_element(results.begin(),
                                              results.end(),
                                              [](Result& a, Result& b) {
                return a.FunctionValue < b.FunctionValue;
            }));
            break;
        }
        case DIFFERENCE:
            float negF;
            for (int i = 1; i < results.size(); ++i) {
                negF = -results[i].FunctionValue;
                if (negF > results[0].FunctionValue) {
                    results[0].FunctionValue = negF;
                }
            }
            res = std::move(results[0]);
            break;
    }

    if (res.FunctionValue <= 0.0f) {
        result = std::move(res);
        return;
    }

    result.FunctionValue = res.FunctionValue;
    result.IntensityValue = CT::GetTissueOrMaterialTypeByName("Air").CtNumber;
    for (auto& entry: result.ArtifactValueMap) {
        entry.second = 0.0f;
    }
}

float ImplicitStructureCombination::FunctionValue(const double x[3]) {
    if (CtStructures.empty()) {
        vtkErrorMacro("CtStructureList is empty. Cannot evaluate");
        return 0.0f;
    }

    float f;
    switch (OpType) {
        case UNION: {
            auto min = VTK_FLOAT_MAX;
            for (const auto& ctStructure: CtStructures) {
                f = ctStructure->FunctionValue(x);
                if (f < min) {
                    min = f;
                }
            }
            return min;
        }

        case INTERSECTION: {
            auto max = VTK_FLOAT_MIN;
            for (const auto& ctStructure: CtStructures) {
                f = ctStructure->FunctionValue(x);
                if (f < max) {
                    max = f;
                }
            }
            return max;
        }

        case DIFFERENCE:
            float result = CtStructures[0]->FunctionValue(x);
            for (int i = 1; i < CtStructures.size(); ++i) {
                f = -CtStructures[i]->FunctionValue(x);
                if (f > result) {
                    result = f;
                }
            }
            return result;
    }

    return 0.0f;
}

bool ImplicitStructureCombination::CtStructureExists(const CtStructure* structure) {
    return this == structure
            || std::any_of(CtStructures.begin(),
                           CtStructures.end(),
                           [structure](CtStructure* child) {
                return child->CtStructureExists(structure);
            });
}

ImplicitStructureCombination*
ImplicitStructureCombination::FindParentOfCtStructure(CtStructure& ctStructure) {
    for (const auto& structure : CtStructures) {
        if (structure == &ctStructure) {
            return this;
        }
    }

    ImplicitStructureCombination* parent;
    ImplicitStructureCombination* implicitStructureCombination;
    for (const auto& structure : CtStructures) {
        if (structure->IsA("ImplicitStructureCombination")) {
            implicitStructureCombination = dynamic_cast<ImplicitStructureCombination*>(structure);
            parent = implicitStructureCombination->FindParentOfCtStructure(ctStructure);
            if (parent) {
                return parent;
            }
        }
    }

    return nullptr;
}

size_t ImplicitStructureCombination::GetNumberOfChildStructures() {
    return CtStructures.size();
}

void ImplicitStructureCombination::ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer) {
    std::replace(CtStructures.begin(), CtStructures.end(), oldChildPointer, newChildPointer);

    newChildPointer->Register(this);
    oldChildPointer->Delete();
}
