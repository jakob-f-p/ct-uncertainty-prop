#include "CombinedStructure.h"

#include "BasicStructure.h"
#include "SimpleTransform.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include <QLabel>
#include <QComboBox>

#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(CombinedStructure);

void CombinedStructure::PrintSelf(ostream& os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);

    os << indent << "Operator Type: " << GetOperatorTypeName() << "\n";

    os << indent << "Ct Structures:\n";
    auto nextIndent = indent.GetNextIndent();
    for (const auto& ctStructure: ChildStructures) {
        ctStructure->PrintSelf(os, nextIndent);
    }
}

vtkMTimeType CombinedStructure::GetMTime() {
    vtkMTimeType thisMTime = CtStructure::GetMTime();

    std::vector<vtkMTimeType> childrenMTimes(ChildStructures.size());
    std::transform(ChildStructures.begin(), ChildStructures.end(), childrenMTimes.begin(),
                   [](CtStructure* child) { return child->GetMTime(); });
    auto maxMTimeChild = std::max_element(childrenMTimes.begin(), childrenMTimes.end());
    vtkMTimeType maxChildMTime = maxMTimeChild == childrenMTimes.end() ? 0 : *maxMTimeChild;

    return std::max(thisMTime, maxChildMTime);
}


void CombinedStructure::SetTransform(const std::array<std::array<float, 3>, 3>& trs) {
    Transform->SetTranslationRotationScaling(trs);
}

CtStructure::SubType CombinedStructure::GetSubType() const {
    return COMBINED;
}

std::string
CombinedStructure::OperatorTypeToString(CombinedStructure::OperatorType operatorType) {
    switch (operatorType) {
        case OperatorType::UNION:        return "Union";
        case OperatorType::INTERSECTION: return "Intersection";
        case OperatorType::DIFFERENCE:   return "Difference";
        default: {
            qWarning("No string representation for this operator exists");
            return "";
        }
    }
}

void CombinedStructure::SetOperatorType(CombinedStructure::OperatorType operatorType) {
    Operator = operatorType;

    Modified();
}

void CombinedStructure::EvaluateAtPosition(const double x[3], CtStructure::Result& result) {
    TracyCZoneN(evaluateCombinationA, "EvaluateCombinationA", true)
    if (ChildStructures.empty()) {
        vtkErrorMacro("CtStructureList is empty. Cannot evaluate");
        return;
    }

//    TODO: Implement correct artifact evaluation (add artifact values that ignore when they are covered by another Structure)
    const size_t size = ChildStructures.size();
    auto results = std::vector<Result>(size);
    TracyCZoneEnd(evaluateCombinationA)

    for (int i = 0; i < ChildStructures.size(); ++i) {
        ChildStructures[i]->EvaluateAtPosition(x, results[i]);
    }

    TracyCZoneN(evaluateCombinationB, "EvaluateCombinationB", true)
    Result res;
    switch (Operator) {
        case OperatorType::UNION: {
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

        case OperatorType::INTERSECTION: {
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
        case OperatorType::DIFFERENCE:
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
CombinedStructure::EvaluateImplicitModel(const double x[3]) const {
    if (ChildStructures.empty()) {
        vtkErrorMacro("CtStructureList is empty. Cannot calculate function value and radiodensity");
        return {};
    }

    double transformedPoint[3];
    Transform->TransformPoint(x, transformedPoint);

    switch (Operator) {
        case OperatorType::UNION: {
            ModelingResult min { VTK_FLOAT_MAX, VTK_FLOAT_MAX };
            for (const auto& ctStructure: ChildStructures) {
                ModelingResult current = ctStructure->EvaluateImplicitModel(transformedPoint);
                if (current.FunctionValue < min.FunctionValue) {
                    min = current;
                }
            }
            return min;
        }

        case OperatorType::INTERSECTION: {
            ModelingResult max { VTK_FLOAT_MIN, VTK_FLOAT_MIN };
            for (const auto& ctStructure: ChildStructures) {
                ModelingResult current = ctStructure->EvaluateImplicitModel(transformedPoint);
                if (current.FunctionValue > max.FunctionValue) {
                    max = current;
                }
            }
            return max;
        }

        case OperatorType::DIFFERENCE: {
            ModelingResult result = ChildStructures[0]->EvaluateImplicitModel(transformedPoint);
            for (int i = 1; i < ChildStructures.size(); ++i) {
                float functionValue = -ChildStructures[i]->FunctionValue(transformedPoint);
                if (functionValue > result.FunctionValue) {
                    result.FunctionValue = functionValue;
                }
            }
            return result;
        }

        default: return {};
    }
}

float CombinedStructure::FunctionValue(const double x[3]) const {
    if (ChildStructures.empty()) {
        vtkErrorMacro("CtStructureList is empty. Cannot evaluate");
        return 0.0f;
    }

    float f;
    switch (Operator) {
        case OperatorType::UNION: {
            auto min = VTK_FLOAT_MAX;
            for (const auto& ctStructure: ChildStructures) {
                f = ctStructure->FunctionValue(x);
                if (f < min) {
                    min = f;
                }
            }
            return min;
        }

        case OperatorType::INTERSECTION: {
            auto max = VTK_FLOAT_MIN;
            for (const auto& ctStructure: ChildStructures) {
                f = ctStructure->FunctionValue(x);
                if (f < max) {
                    max = f;
                }
            }
            return max;
        }

        case OperatorType::DIFFERENCE: {
            float result = ChildStructures[0]->FunctionValue(x);
            for (int i = 1; i < ChildStructures.size(); ++i) {
                f = -ChildStructures[i]->FunctionValue(x);
                if (f > result) {
                    result = f;
                }
            }
            return result;
        }

        default: return 0.0f;
    }
}

void CombinedStructure::AddCtStructure(CtStructure& ctStructure) {
    if (auto search = std::find(ChildStructures.begin(), ChildStructures.end(), &ctStructure);
            search != ChildStructures.end()) {
        vtkWarningMacro("Trying to add implicit CT Structure which is already present");
    }

    ChildStructures.emplace_back(&ctStructure);
    ctStructure.SetParent(this);

    Modified();
}

CtStructure* CombinedStructure::RemoveBasicStructure(BasicStructure* basicStructure,
                                                     CombinedStructure* grandParent) {
    auto removeIt = std::find(ChildStructures.begin(), ChildStructures.end(), basicStructure);
    if (removeIt == ChildStructures.end()) {
        vtkWarningMacro("Given Structure could not be removed because it was not present");
        return nullptr;
    }

    ChildStructures.erase(removeIt);


    if (ChildStructures.size() == 1) {
        auto remainingStructure = ChildStructures[0];

        if (!grandParent) {     // this node is root
            return remainingStructure;
        }

        remainingStructure->SetParent(grandParent);
        grandParent->ReplaceConnection(this, remainingStructure);
    }

    return nullptr;
}

bool CombinedStructure::CtStructureExists(const CtStructure* structure) {
    return this == structure
            || std::any_of(ChildStructures.begin(),
                           ChildStructures.end(),
                           [structure](CtStructure* child) {
                return child->CtStructureExists(structure);
            });
}

int CombinedStructure::ChildCount() const {
    return static_cast<int>(ChildStructures.size());
}

const CtStructure* CombinedStructure::ChildAt(int idx) const {
    return ChildStructures.at(idx);
}

int CombinedStructure::ChildIndex(const CombinedStructure& child) const {
    auto searchIt = std::find(ChildStructures.begin(), ChildStructures.end(), &child);
    if(searchIt == ChildStructures.end())
        qWarning("Given Structure is not contained in children");

    return static_cast<int>(std::distance(ChildStructures.begin(), searchIt));
}

std::string CombinedStructure::GetViewName() const {
    return GetOperatorTypeName() + (Name.empty() ? "" : " (" + Name + ")");
}

std::string CombinedStructure::GetOperatorTypeName() const {
    return OperatorTypeToString(Operator);
}

void CombinedStructure::ReplaceChild(BasicStructure* oldChild, CombinedStructure* newChild) {
    auto oldIt = std::find(ChildStructures.begin(), ChildStructures.end(), oldChild);
    if (oldIt == ChildStructures.end()){
        qWarning("Given pointer to old child is not a child of this Structure");
        return;
    }

    *oldIt = newChild;
}

void CombinedStructure::Iterate(const std::function<void(CtStructure&)>& f) {
    f(*this);

    std::for_each(ChildStructures.begin(), ChildStructures.end(),
                  [&](CtStructure* structure) { f(*structure); });
}

void CombinedStructure::ReplaceConnection(CtStructure* oldChildPointer, CtStructure* newChildPointer) {
    std::replace(ChildStructures.begin(), ChildStructures.end(), oldChildPointer, newChildPointer);

    newChildPointer->Register(this);
    oldChildPointer->Delete();
}

void CombinedStructureData::AddSubTypeData(const CombinedStructure& structure, CombinedStructureData& data) {
    data.Operator = structure.Operator;
}

void CombinedStructureData::SetSubTypeData(CombinedStructure& structure, const CombinedStructureData& data) {
    structure.Operator = data.Operator;
}

void CombinedStructureUi::AddSubTypeWidgets(QFormLayout* fLayout) {
    auto* operatorTypeComboBox = new QComboBox();
    operatorTypeComboBox->setObjectName(OperatorTypeComboBoxName);
    for (const auto &operatorAndName : CombinedStructure::GetOperatorTypeValues()) {
        operatorTypeComboBox->addItem(operatorAndName.Name, QVariant::fromValue(operatorAndName.EnumValue));
    }
    fLayout->addRow("Operator Type", operatorTypeComboBox);
}

void CombinedStructureUi::AddSubTypeWidgetsData(QWidget* widget, CombinedStructureData& data) {
    auto* operatorTypeComboBox = widget->findChild<QComboBox*>(OperatorTypeComboBoxName);

    data.Operator = operatorTypeComboBox->currentData().value<CombinedStructure::OperatorType>();
}

void CombinedStructureUi::SetSubTypeWidgetsData(QWidget* widget, const CombinedStructureData& data) {
    auto* operatorTypeComboBox = widget->findChild<QComboBox*>(OperatorTypeComboBoxName);

    if (int idx = operatorTypeComboBox->findData(QVariant::fromValue(data.Operator));
            idx != -1) {
        operatorTypeComboBox->setCurrentIndex(idx);
    }
}

const QString CombinedStructureUi::OperatorTypeComboBoxName = "OperatorType";
