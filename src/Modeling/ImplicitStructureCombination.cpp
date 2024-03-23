#include "ImplicitStructureCombination.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(ImplicitStructureCombination);

void ImplicitStructureCombination::PrintSelf(ostream& os, vtkIndent indent) {
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

    std::vector<vtkMTimeType> childrenMTimes(CtStructures.size());
    std::transform(CtStructures.begin(), CtStructures.end(), childrenMTimes.begin(),
                   [](CtStructure* child) { return child->GetMTime(); });
    auto maxMTimeChild = std::max_element(childrenMTimes.begin(), childrenMTimes.end());
    vtkMTimeType maxChildMTime = maxMTimeChild == childrenMTimes.end() ? 0 : *maxMTimeChild;

    return std::max(thisMTime, maxChildMTime);
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

void ImplicitStructureCombination::SetOperatorType(ImplicitStructureCombination::OperatorType operatorType) {
    this->OpType = operatorType;

    this->Modified();
}

ImplicitStructureCombination::OperatorType ImplicitStructureCombination::GetOperatorType() const {
    return this->OpType;
}

void ImplicitStructureCombination::SetTransform(const std::array<std::array<float, 3>, 3>& trs) {
    this->Transform->SetTranslationRotationScaling(trs);
}

void ImplicitStructureCombination::EvaluateAtPosition(const double x[3], CtStructure::Result& result) {
    TracyCZoneN(evaluateCombinationA, "EvaluateCombinationA", true)
    if (CtStructures.empty()) {
        vtkErrorMacro("CtStructureList is empty. Cannot evaluate");
        return;
    }

//    TODO: Implement correct artifact evaluation (add artifact values that ignore when they are covered by another structure)
    const size_t size = CtStructures.size();
    auto results = std::vector<Result>(size);
    TracyCZoneEnd(evaluateCombinationA)

    for (int i = 0; i < CtStructures.size(); ++i) {
        CtStructures[i]->EvaluateAtPosition(x, results[i]);
    }

    TracyCZoneN(evaluateCombinationB, "EvaluateCombinationB", true)
    Result res;
    switch (OpType) {
        case UNION: {
            const Result* minRes;
            auto minF = VTK_FLOAT_MAX;
            for (int i = 0; i < size; ++i) {
                float f = results[i].FunctionValue;
                if (f <= minF) {
                    minF = f;
                    minRes = &results[i];
                }
            }
            res = *minRes;
//            res = std::move(*std::min_element(results.begin(),
//                                              results.end(),
//                                              [](Result& a, Result& b) {
//                return a.FunctionValue < b.FunctionValue;
//            }));
            break;
        }

        case INTERSECTION: {
            const Result* maxRes;
            auto maxF = VTK_FLOAT_MIN;
            for (int i = 0; i < size; ++i) {
                float f = results[i].FunctionValue;
                if (f >= maxF) {
                    maxF = f;
                    maxRes = &results[i];
                }
            }
            res = *maxRes;
//            res = std::move(*std::max_element(results.begin(),
//                                              results.end(),
//                                              [](Result& a, Result& b) {
//                return a.FunctionValue < b.FunctionValue;
//            }));
            break;
        }
        case DIFFERENCE:
            for (int i = 1; i < size; ++i) {
                float negF = -results[i].FunctionValue;
                if (negF > results[0].FunctionValue) {
                    results[0].FunctionValue = negF;
                }
            }
            res = std::move(results[0]);
            break;
        default: break;
    }

    if (res.FunctionValue <= 0.0f) {
        result = std::move(res);
        return;
    }

    result.FunctionValue = res.FunctionValue;
    result.IntensityValue = res.IntensityValue;
    for (auto& entry: result.ArtifactValueMap) {
        entry.second = 0.0f;
    }
    TracyCZoneEnd(evaluateCombinationB)
}

const CtStructure::ModelingResult
ImplicitStructureCombination::EvaluateImplicitModel(const double x[3]) const {
    if (CtStructures.empty()) {
        vtkErrorMacro("CtStructureList is empty. Cannot calculate function value and radiodensity");
        return {};
    }

    double transformedPoint[3];
    Transform->TransformPoint(x, transformedPoint);

    switch (OpType) {
        case UNION: {
            ModelingResult min { VTK_FLOAT_MAX, VTK_FLOAT_MAX };
            for (const auto* ctStructure: CtStructures) {
                ModelingResult current = ctStructure->EvaluateImplicitModel(transformedPoint);
                if (current.FunctionValue < min.FunctionValue) {
                    min = current;
                }
            }
            return min;
        }

        case INTERSECTION: {
            ModelingResult max { VTK_FLOAT_MIN, VTK_FLOAT_MIN };
            for (const auto* ctStructure: CtStructures) {
                ModelingResult current = ctStructure->EvaluateImplicitModel(transformedPoint);
                if (current.FunctionValue > max.FunctionValue) {
                    max = current;
                }
            }
            return max;
        }

        case DIFFERENCE: {
            ModelingResult result = CtStructures[0]->EvaluateImplicitModel(transformedPoint);
            for (int i = 1; i < CtStructures.size(); ++i) {
                float functionValue = -CtStructures[i]->FunctionValue(transformedPoint);
                if (functionValue > result.FunctionValue) {
                    result.FunctionValue = functionValue;
                }
            }
            return result;
        }

        default: return {};
    }
}

float ImplicitStructureCombination::FunctionValue(const double x[3]) const {
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

        case DIFFERENCE: {
            float result = CtStructures[0]->FunctionValue(x);
            for (int i = 1; i < CtStructures.size(); ++i) {
                f = -CtStructures[i]->FunctionValue(x);
                if (f > result) {
                    result = f;
                }
            }
            return result;
        }

        default: return 0.0f;
    }
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

QVariant ImplicitStructureCombination::Data() const {
    ImplicitStructureCombinationDetails combinationDetails {
        GetCtStructureDetails(),
        OpType
    };
    return QVariant::fromValue(combinationDetails);
}

ImplicitStructureCombination::ImplicitStructureCombination() {
    this->OpType = UNION;
}

ImplicitStructureCombination::~ImplicitStructureCombination() {
    for (const auto& structure : CtStructures) {
        structure->Delete();
    }
}

std::string ImplicitStructureCombination::GetViewName() const {
    return GetOperatorTypeName() + (Name.empty() ? "" : " (" + Name + ")");
}

std::string ImplicitStructureCombination::GetOperatorTypeName() const {
    return OperatorTypeToString(OpType);
}

void ImplicitStructureCombination::SetData(const QVariant &variant) {
    auto implicitStructureCombinationDetails = variant.value<ImplicitStructureCombinationDetails>();

    SetCtStructureDetails(implicitStructureCombinationDetails);

    SetOperatorType(implicitStructureCombinationDetails.OperatorType);
}

bool ImplicitStructureCombination::IsImplicitCtStructure() const {
    return false;
}

void ImplicitStructureCombination::DeepCopy(CtStructure* source, CtStructure* parent) {
    Superclass::DeepCopy(source, parent);

    const auto* implicitStructureCombinationSource = dynamic_cast<ImplicitStructureCombination*>(source);
    OpType = implicitStructureCombinationSource->OpType;

    for (const auto &ctStructure: CtStructures) {
        ctStructure->Delete();
    }
    CtStructures.clear();

    for (const auto &ctStructure: implicitStructureCombinationSource->CtStructures) {
        CtStructure* newCtStructure = ctStructure->IsImplicitCtStructure()
                ? static_cast<CtStructure*>(ImplicitCtStructure::New())
                : static_cast<CtStructure*>(ImplicitStructureCombination::New());
        newCtStructure->DeepCopy(ctStructure, this);
        CtStructures.push_back(newCtStructure);
    }
}

void ImplicitStructureCombination::ReplaceChild(ImplicitCtStructure *oldChild, ImplicitStructureCombination *newChild) {
    auto oldStructure = std::find(CtStructures.begin(), CtStructures.end(), oldChild);
    if (oldStructure == CtStructures.end()){
        qWarning("Given pointer to old child is not a child of this structure");
        return;
    }

    *oldStructure = newChild;

    oldChild->UnRegister(this);
    newChild->Register(this);
}
