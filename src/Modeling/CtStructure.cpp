#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include "ImplicitCtStructure.h"
#include "CtStructure.h"

#include <utility>

void CtStructure::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

void CtStructure::SetName(std::string name) {
    Name = std::move(name);
}
std::string CtStructure::GetName() const {
    return Name;
}

const SimpleTransform* CtStructure::GetTransform() const {
    return Transform;
}

QVariant CtStructure::GetTransformQVariant() const {
    return Transform->GetTranslationRotationScaling();
}

std::string CtStructure::DataKeyToString(CtStructure::DataKey dataKey) {
    return std::to_string(dataKey);
}

CtStructure::DataKey CtStructure::StringToDataKey(const std::string& string) {
    DataKey dataKey;
    for (int i = 0; i < NUMBER_OF_DATA_KEYS; ++i) {
        dataKey = static_cast<DataKey>(i);
        if (DataKeyToString(dataKey) == string) {
            return dataKey;
        }
    }

    qWarning("no matching data key found");
    return NUMBER_OF_DATA_KEYS;
}

int CtStructure::ColumnCount() {
    return 1;
}

CtStructure* CtStructure::GetParent() const {
    return Parent;
}

void CtStructure::SetParent(CtStructure* parent) {
    Parent = parent;
}

int CtStructure::ChildIndex() const {
    if (!Parent) {
        return 0;
    }

    auto* childrenOfParent = Parent->GetChildren();
    if (!childrenOfParent) {
        return -1;
    }

    auto searchIt = std::find(childrenOfParent->begin(), childrenOfParent->end(), this);

    assert(searchIt != childrenOfParent->end());

    return static_cast<int>(std::distance(childrenOfParent->begin(), searchIt));
}

CtStructure::CtStructure() {
    Parent = nullptr;
    Transform = SimpleTransform::New();
}

CtStructure::~CtStructure() {
    Transform->Delete();
}

QVariant CtStructure::Data() const {
    QMap<QString, QVariant> map;
    DataKey dataKey;
    QVariant val;

    for (int i = 0; i < NUMBER_OF_DATA_KEYS; ++i) {
        dataKey = static_cast<DataKey>(i);
        val = PackageData(dataKey);
        if (val.isValid()) {
            map[DataKeyToString(dataKey).c_str()] = val;
        }
    }

    return map;
}

