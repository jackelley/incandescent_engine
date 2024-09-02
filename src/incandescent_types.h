#ifndef INCANDESCENT_TYPES
#define INCANDESCENT_TYPES


#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>

#define VK_NO_PROTOTYPES
#ifdef __APPLE__
#include <MoltenVK/mvk_vulkan.h>
#include <MoltenVK/mvk_config.h>
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL.h>
#include <volk.h>

#include <fmt/core.h>
#include <Eigen/Dense>

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            fmt::println("Detected Vulkan error: {}", string_VkResult(err)); \
            abort();                                                    \
        }                                                               \
    } while (0)

#endif