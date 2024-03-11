#pragma once

#include <algorithm>
#include <map>
#include <vector>

#include "StructureArtifact.h"

class StructureArtifactList : public vtkObject {
public:
    static StructureArtifactList* New();

    void AddStructureArtifact(StructureArtifact* structureArtifact);

    typedef std::map<Artifact::SubType, std::vector<StructureArtifact*>> TypeToStructureArtifactsMap;

    TypeToStructureArtifactsMap GetStructureArtifactMap();

    StructureArtifactList(const StructureArtifactList&) = delete;
    void operator=(const StructureArtifactList&) = delete;

protected:
    StructureArtifactList() = default;
    ~StructureArtifactList() override = default;

private:
    std::vector<StructureArtifact*> Artifacts;
};