#pragma once

#include <vtkTimeStamp.h>

#include <optional>
#include <vector>


template<typename DataT>
requires std::is_same_v<std::decay_t<DataT>, DataT> && (!std::is_pointer_v<std::decay_t<DataT>>)
struct TimeStampedData {
    TimeStampedData() = default;
    explicit TimeStampedData(DataT&& data, vtkMTimeType time) :
            OptionalData(std::move(data)), Time(time) {}

    [[nodiscard]] auto
    GetTime() const noexcept -> vtkMTimeType { return Time; }

    [[nodiscard]] auto
    IsUpToDateWith(vtkTimeStamp other) const noexcept -> bool { return other <= Time; }

    template<typename... Args>
    auto
    Emplace(Args&&... args) noexcept -> DataT& {
        vtkTimeStamp timeStamp;
        timeStamp.Modified();
        Time = timeStamp;
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
    vtkMTimeType Time = 0;
    std::optional<DataT> OptionalData;
};


template<typename DataT>
requires std::is_same_v<std::decay_t<DataT>, DataT> && (!std::is_pointer_v<std::decay_t<DataT>>)
struct TimeStampedDataRef {
    explicit TimeStampedDataRef(TimeStampedData<DataT> const& data) :
            DataRef(*data),
            Time(data.GetTime()) {};

    [[nodiscard]] auto
    GetTime() const noexcept -> vtkMTimeType { return Time; }

    [[nodiscard]] auto
    IsUpToDateWith(vtkTimeStamp other) const noexcept -> bool { return other <= Time; }

    constexpr auto
    operator*() const& -> DataT const& { return DataRef; }

    constexpr auto
    operator->() const -> DataT const* { return std::addressof(DataRef); }

private:
    DataT const& DataRef;
    vtkMTimeType Time;
};
