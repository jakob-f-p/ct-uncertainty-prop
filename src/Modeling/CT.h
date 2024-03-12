#pragma once

#include <iostream>
#include <map>
#include <string>

namespace CT {

    struct TissueOrMaterialType {
        std::string Name;
        float CtNumber; // value on the Hounsfield scale

        friend std::ostream& operator<< (std::ostream& stream, const TissueOrMaterialType& type);
    };

    typedef std::map<std::string, TissueOrMaterialType> NameTissueTypeMap;

    namespace {
        #define FOR_EACH_TISSUE_OR_MATERIAL_TYPE(DO) \
                DO(Air,          -1000.0f)           \
                DO(Fat,           -100.0f)           \
                DO(Water,            0.0f)           \
                DO(SoftTissue,     200.0f)           \
                DO(CancellousBone, 350.0f)           \
                DO(CorticalBone,   800.0f)           \
                DO(Metal,        15000.0f)

        #define MAP_INITIALIZER(name, ctNumber) { #name, { #name, ctNumber } },

        const NameTissueTypeMap defaultTissueTypes = {
                FOR_EACH_TISSUE_OR_MATERIAL_TYPE(MAP_INITIALIZER)
        };
    }

    #define ENUM_DEFINITION(name, ctNumber) name,

    enum TissueOrMaterialTypes {
        FOR_EACH_TISSUE_OR_MATERIAL_TYPE(ENUM_DEFINITION)
    };

    NameTissueTypeMap GetDefaultTissueTypes();

    TissueOrMaterialType GetTissueOrMaterialTypeByName(const std::string& tissueName,
                                                       const CT::NameTissueTypeMap& tissueTypeMap = defaultTissueTypes);
}