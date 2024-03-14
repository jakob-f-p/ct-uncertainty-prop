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


void ImplicitStructureCombination::AddCtStructure(CtStructure& ctStructure) {
    if (auto search = std::find(CtStructures.begin(), CtStructures.end(), &ctStructure);
        search != CtStructures.end()) {
        vtkWarningMacro("Trying to add implicit CT structure which is already present");
    }

    CtStructures.push_back(&ctStructure);
    ctStructure.SetParent(this);
    ctStructure.Register(this);

    this->Modified();
}

std::string
ImplicitStructureCombination::OperatorTypeToString(ImplicitStructureCombination::OperatorType operatorType) {
    switch (operatorType) {
        case UNION:        return "Union";
        case INTERSECTION: return "Intersection";
        case DIFFERENCE:   return "Difference";
        default: {
            qWarning("No string representation for this operator exists");
            return "";
        }
    }
}

ImplicitStructureCombination::OperatorType
ImplicitStructureCombination::StringToOperatorType(const std::string& string) {
    OperatorType operatorType;
    for (int i = 0; i < NUMBER_OF_OPERATOR_TYPES; ++i) {
        operatorType = static_cast<OperatorType>(i);
        if (OperatorTypeToString(operatorType) == string) {
            return operatorType;
        }
    }

    qWarning("no matching operator type found");
    return NUMBER_OF_OPERATOR_TYPES;

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

        remainingStructure->SetParent(grandParent);
        grandParent->ReplaceConnection(this, remainingStructure);
    }

    return nullptr;
}

ImplicitStructureCombination::ImplicitStructureCombination() {
    this->OpType = UNION;
}

ImplicitStructureCombination::~ImplicitStructureCombination() {
    for (const auto& structure : CtStructures) {
        structure->UnRegister(this);
    }
}

void ImplicitStructureCombination::SetOperatorType(ImplicitStructureCombination::OperatorType operatorType) {
    this->OpType = operatorType;
}

ImplicitStructureCombination::OperatorType ImplicitStructureCombination::GetOperatorType() const {
    return this->OpType;
}

std::string ImplicitStructureCombination::GetOperatorTypeName() const {
    switch (OpType) {
        case UNION:        return "Union";
        case INTERSECTION: return "Intersection";
        case DIFFERENCE:   return "Difference";
    }

    return "";
}

void ImplicitStructureCombination::SetTransform(const QVariant& trs) {
    this->Transform->SetTranslationRotationScaling(trs);
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
    result.IntensityValue = ImplicitCtStructure::GetTissueOrMaterialTypeByName("Air").CtNumber;
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

void ImplicitStructureCombination::ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer) {
    std::replace(CtStructures.begin(), CtStructures.end(), oldChildPointer, newChildPointer);

    newChildPointer->Register(this);
    oldChildPointer->Delete();
}

int ImplicitStructureCombination::ChildCount() const {
    return static_cast<int>(CtStructures.size());
}

const std::vector<CtStructure*>* ImplicitStructureCombination::GetChildren() const {
    return &CtStructures;
}

const CtStructure* ImplicitStructureCombination::ChildAt(int idx) const {
    if (idx >= CtStructures.size()) {
        vtkErrorMacro("Index is not in range");
        return nullptr;
    }

    return CtStructures[idx];
}

QVariant ImplicitStructureCombination::PackageData(CtStructure::DataKey dataKey) const {
    switch (dataKey) {
        case NAME: return Name.c_str();
        case EDIT_DIALOG_NAME: return ("Combination" + (Name.empty() ? "" : " (" + Name + ")")).c_str();
        case TREE_VIEW_NAME: return (GetOperatorTypeName() + (Name.empty() ? "" : " (" + Name + ")")).c_str();
        case TRANSFORM: return GetTransformQVariant();
        case OPERATOR_TYPE: return static_cast<int>(OpType);
        default: return {};
    }
}
