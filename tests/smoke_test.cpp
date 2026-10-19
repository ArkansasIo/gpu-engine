#include "gpu_engine/gpu_engine.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    const auto info = gpu_engine::get_engine_info();
    if (info.name.empty()) {
        std::cerr << "Engine name must not be empty\n";
        return EXIT_FAILURE;
    }
    if (info.api_major != 0 || info.api_minor != 1 || info.api_patch != 0) {
        std::cerr << "Unexpected API version\n";
        return EXIT_FAILURE;
    }
    if (gpu_engine::backend_name(gpu_engine::Backend::Vulkan) == nullptr) {
        std::cerr << "Backend label must not be null\n";
        return EXIT_FAILURE;
    }
    if (!gpu_engine::enumerate_devices().empty()) {
        std::cerr << "Expected no devices before implementing a backend\n";
        return EXIT_FAILURE;
    }
    const auto success = gpu_engine::Status::success();
    const auto failure = gpu_engine::Status::failure("expected test failure");
    if (!success.ok || failure.ok || failure.message.empty()) {
        std::cerr << "Status invariants failed\n";
        return EXIT_FAILURE;
    }
    std::cout << info.name << " " << info.api_major << "." << info.api_minor
              << "." << info.api_patch << " smoke test passed\n";
    return EXIT_SUCCESS;
}
