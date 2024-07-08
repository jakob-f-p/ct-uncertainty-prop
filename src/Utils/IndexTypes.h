#pragma once

#include <optional>
#include <stdexcept>


using uidx_t = uint16_t;


using StructureId = uint16_t;


struct idx_t : public std::optional<uidx_t> {
    using std::optional<uidx_t>::operator=;

    auto
    operator++() noexcept -> idx_t& { if (has_value()) value()++; return *this;  }

    auto
    operator++(int) noexcept -> idx_t { idx_t previous = *this; ++(*this); return previous;  }

    auto
    operator--() noexcept -> idx_t& { if (has_value()) value()--; return *this;  }

    auto
    operator--(int) noexcept -> idx_t { idx_t previous = *this; --(*this); return previous;  }

    [[nodiscard]] auto
    ToSigned() const noexcept -> int32_t { return has_value() ? value() : -1; }

    [[nodiscard]] auto
    ToUnsigned() const -> uidx_t {
        if (!has_value())
            throw std::runtime_error("idx must be valid");

        return value();
    }

    [[nodiscard]] auto static
    FromSigned(int32_t idx) noexcept -> idx_t { return idx >= 0 ? idx_t { idx } : idx_t { std::nullopt }; }

    [[nodiscard]] auto static
    SignedAsUnsignedIdx(int32_t idx) -> uidx_t {
        if (idx < 0)
            throw std::runtime_error("idx must be positive");

        return static_cast<uidx_t>(idx);
    }
};

