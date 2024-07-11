#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>

#ifdef UP_WINDOWS
#include <windows.h>
#endif

#ifdef UP_UNIX
#include <unistd.h>
#endif


namespace System {

    enum struct Architecture : uint8_t {
        BIT32,
        BIT64
    };

    enum struct OperatingSystem : uint8_t {
        WINDOWS,
        UNIX
    };

    static constexpr uint64_t KiloByte = 1 << 10;
    static constexpr uint64_t MegaByte = KiloByte << 10;
    static constexpr uint64_t GigaByte = MegaByte << 10;
    static constexpr uint64_t TeraByte = GigaByte << 10;

    [[nodiscard]] consteval auto
    GetArchitectureType() -> Architecture {
        switch (sizeof(void*)) {
            case 4: return Architecture::BIT32;
            case 8: return Architecture::BIT64;
            default: throw std::runtime_error("unsupported architecture type");
        }
    }

    [[nodiscard]] consteval auto
    GetOSType() -> OperatingSystem {
#ifdef UP_WINDOWS
        return OperatingSystem::WINDOWS;
#endif

#ifdef UP_UNIX
        return OperatingSystem::UNIX;
#endif

        throw std::runtime_error("unsupported operating system");
    }

    [[nodiscard]] auto
    GetTotalSystemMemory() -> uint64_t {
#ifdef UP_WINDOWS
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;
#endif

#ifdef UP_UNIX
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
#endif

        throw std::runtime_error("unsupported operating system");
    }

    [[nodiscard]] auto
    GetMaxApplicationMemory() -> uint64_t {
        uint64_t const totalMemory = GetTotalSystemMemory();
        uint64_t const halfTotalMemory = totalMemory / 2;

        if (GetArchitectureType() == Architecture::BIT32)
            return std::min(halfTotalMemory, 2 * GigaByte);

        return std::min(halfTotalMemory, 16 * TeraByte);
    }
}