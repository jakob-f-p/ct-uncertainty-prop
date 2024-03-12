#include <vtkObject.h>
#include <vtkSetGet.h>

#include "CT.h"

std::ostream& CT::operator<<(ostream& stream, const CT::TissueOrMaterialType& type) {
    return stream << type.Name << ": ('" << type.CtNumber << "')";
}

CT::NameTissueTypeMap CT::GetDefaultTissueTypes() {
    return CT::defaultTissueTypes;
}

CT::TissueOrMaterialType CT::GetTissueOrMaterialTypeByName(const std::string& tissueName,
                                                           const CT::NameTissueTypeMap& tissueTypeMap) {
    if (auto search = tissueTypeMap.find(tissueName); search != tissueTypeMap.end()) {
        return search->second;
    }

    vtkWarningWithObjectMacro(nullptr, "No tissue type with requested name present. Returning 'Air'");

    return defaultTissueTypes.at("Air");
}
