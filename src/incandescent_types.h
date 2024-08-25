// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.
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

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

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