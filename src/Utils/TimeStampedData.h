#pragma once

#include <vtkTimeStamp.h>

#include <optional>
#include <vector>


template<typename DataT>
struct TimeStampedData {
    explicit TimeStampedData() = default;

    [[nodiscard]] auto
    GetTime() const noexcept -> vtkMTimeType { return TimeStamp.GetMTime(); }

    [[nodiscard]] auto
    IsUpToDateWith(vtkTimeStamp other) const noexcept -> bool { return other <= TimeStamp; }

    template<typename... Args>
    auto constexpr
    Emplace(Args&&... args) noexcept -> DataT& {
        TimeStamp.Modified();
        return OptionalData.emplace(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept { return static_cast<bool>(OptionalData); }

    constexpr auto
    operator*() const& -> DataT const& {
        if (!OptionalData)
            throw std::runtime_error("data empty");

        return *OptionalData;
    }

    constexpr auto
    operator*() & -> DataT& {
        if (!OptionalData)
            throw std::runtime_error("data empty");

        return *OptionalData;
    }

    constexpr auto
    operator->() const -> DataT const* {
        if (!OptionalData)
            throw std::runtime_error("data empty");

        return std::addressof(*OptionalData);
    }

    constexpr auto
    operator->() -> DataT* {
        if (!OptionalData)
            throw std::runtime_error("data empty");

        return std::addressof(*OptionalData);
    }

private:
    vtkTimeStamp TimeStamp;
    std::optional<DataT> OptionalData;
};
