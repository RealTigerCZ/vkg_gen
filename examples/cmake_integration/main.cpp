#include "vkg.hpp"

#include <cstdio>

int main() {
    // Test that vkg generated header is included
    vk::Instance instance{};
    vk::SwapchainKHR swapchain{};
    std::printf("vkg generated header included successfully\n");
    return 0;
}
