//
// Created by namus on 2/5/2026.
//
#pragma once

#include <iostream>
#include <volk/volk.h>
#include <stdexcept>

static inline void chk(VkResult result) {
    if (result != VK_SUCCESS) {
        std::cerr << "Vulkan call returned an error (" << result << ")\n";
        exit(result);
    }
}

static inline void chk(bool result) {
    if (!result) {
        std::cerr << "Call returned an error\n";
        exit(result);
    }
}