#pragma once

#include <array>

#include <vtkType.h>

[[nodiscard]] auto static
PointIdToDimensionCoordinates(vtkIdType pointId,
                              std::array<int, 3> const dimensions) -> std::array<int, 3> {
    int const dimX = dimensions[0];
    int const dimX_dimY = dimensions[0] * dimensions[1];
    int const z = pointId / dimX_dimY;
    int const y = (pointId - z * dimX_dimY) / dimX;
    int const x = pointId - z * dimX_dimY - y * dimX;
    return { x, y, z };
}

[[nodiscard]] auto static
GetDecrementedCoordinates(std::array<int, 3> const coordinates,
                          std::array<int, 3> const dimensions) -> std::array<int, 3> {
    std::array<int, 3> decrementedCoordinates = coordinates;

    if (coordinates[0] == 0) {
        decrementedCoordinates[0] = dimensions[0] - 1;

        if (coordinates[1] == 0) {
            decrementedCoordinates[1] = dimensions[1] - 1;

            decrementedCoordinates[2] = coordinates[2] - 1;
        } else {
            decrementedCoordinates[1] = coordinates[1] - 1;
        }
    } else {
        decrementedCoordinates[0] = coordinates[0] - 1;
    }

    return decrementedCoordinates;
}

