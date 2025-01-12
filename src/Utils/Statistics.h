#pragma once

#include <numeric>
#include <span>
#include <vector>


namespace Stats {


template<typename T>
concept NumberT = std::is_arithmetic_v<T>;


template<typename T>
[[nodiscard]] auto
Mean(std::span<T> values) noexcept -> T {
    T const sum = std::reduce(values.begin(), values.end());
    return sum / values.size();
}

template<typename T>
[[nodiscard]] auto
Variance(std::span<T> values) noexcept -> T {
    T const mean = Mean(values);
    T const sumOfSquaredDifferences = std::reduce(values.begin(), values.end(),
                                                  0.0, [mean](T, T const& val) {
        T const& diff = val - mean;
        return diff * diff;
    });
    return sumOfSquaredDifferences / values.size();
}

template<typename T>
[[nodiscard]] auto
CumulativeSum(std::vector<T> values) noexcept -> std::vector<T> {
    std::vector<T> cumulativeSumVector { values.size(), std::allocator<T> {} };

    if (cumulativeSumVector.empty())
        return cumulativeSumVector;

    cumulativeSumVector[0] = values[0];
    for (size_t i = 1; i < cumulativeSumVector.size(); ++i)
        cumulativeSumVector[i] = cumulativeSumVector[i - 1] + values[i];

    return cumulativeSumVector;
}


}