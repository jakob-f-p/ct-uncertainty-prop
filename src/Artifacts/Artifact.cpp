#include "Artifact.h"

void Artifact::PrintSelf(std::ostream& os, vtkIndent indent) {
    this->Superclass::PrintSelf(os, indent);

    os << indent << "Name: '" << Name << "'\n";
}

std::string Artifact::GetName() {
    return Name;
}
