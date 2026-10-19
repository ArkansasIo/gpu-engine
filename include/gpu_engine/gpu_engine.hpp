#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gpu_engine {

constexpr std::uint32_t api_version_major = 0;
constexpr std::uint32_t api_version_minor = 1;
constexpr std::uint32_t api_version_patch = 0;

enum class Backend : std::uint8_t { None, Vulkan, Direct3D12, Metal, WebGPU };

struct DeviceCapabilities {
    std::string name;
    Backend backend{Backend::None};
    std::uint32_t vendor_id{0};
    std::uint32_t device_id{0};
    std::uint64_t dedicated_memory_bytes{0};
    bool graphics_supported{false};
    bool compute_supported{false};
    bool presentation_supported{false};
};

struct Status {
    bool ok{true};
    std::string message;
    static Status success();
    static Status failure(std::string message);
};

struct EngineInfo {
    std::string name{"GPU Engine"};
    std::uint32_t api_major{api_version_major};
    std::uint32_t api_minor{api_version_minor};
    std::uint32_t api_patch{api_version_patch};
};

/// Return compile-time engine metadata; no GPU SDK or device is required.
EngineInfo get_engine_info();

/// Returns empty until a hardware backend adapter is implemented.
std::vector<DeviceCapabilities> enumerate_devices();

/// Convert a backend identifier to a stable human-readable label.
const char* backend_name(Backend backend) noexcept;

} // namespace gpu_engine
