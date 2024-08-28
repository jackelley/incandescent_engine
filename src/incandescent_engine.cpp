//
// Created by Jack Kelley on 8/24/24.
//
#include <incandescent_types.h>
#include <incandescent_engine.h>

#include <chrono>
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define VOLK_IMPLEMENTATION
#include <fstream>
#include <iostream>
#include <queue>
#include <volk.h>

#ifdef __APPLE__
#ifndef VK_USE_PLATFORM_METAL_EXT
#define VK_USE_PLATFORM_METAL_EXT
#endif
#endif

constexpr bool use_validation_layers = true;
constexpr bool use_log_file = true;

IncandescentEngine *loaded_engine = nullptr;

IncandescentEngine &IncandescentEngine::Get() {
    return *loaded_engine;
}

void IncandescentEngine::initialize() {
    // Create logfile
    std::ofstream log_file;
    log_file.open("./src/log_file.txt");
    log_file << "Created log " << std::chrono::system_clock::now() << "\n";
    log_file.close();

    // Initialize volk
    VkResult volk_init = volkInitialize();
    if (volk_init != VK_SUCCESS) {
        fmt::print("Vulkan loader failed!");
    }
    if (use_log_file) {
        log_file.open("./src/log_file.txt", std::ios_base::app);
        log_file << "Volk initialized\n";
        log_file.close();
    }

    // Make sure that there isn't already an initialized engine
    assert(loaded_engine == nullptr);
    loaded_engine = this;

    // Create the window
    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = (SDL_WINDOW_VULKAN);

    window = SDL_CreateWindow("Incandescent Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
    if (window == nullptr) {
        throw std::runtime_error("Window not initialized!");
    }
    if (use_log_file) {
        log_file.open("./src/log_file.txt", std::ios_base::app);
        log_file << "Window initialized\n";
        log_file.close();
    }

    initialize_vulkan();

    initialize_swapchain(WIDTH, HEIGHT);

    initialize_commands();

    initialize_sync_structures();

    // Set success check bool to true
    is_initialized = true;
}

void IncandescentEngine::initialize_vulkan() {
    /* -------- Instance -------- */
    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = nullptr;
    application_info.pApplicationName = "Incandescent v0.1";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    // Activate extensions portability bit for MoltenVK (funny mac)
    std::vector<const char *> instance_extension_names;
    instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance_extension_names.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    instance_extension_names.push_back("VK_KHR_surface");

    // Activate layers
    const std::vector<const char *> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Get SDL needed extensions
    uint32_t sdl_extensions_count;
    SDL_Vulkan_GetInstanceExtensions(window, &sdl_extensions_count, nullptr);
    std::vector<const char *> sdl_extensions(sdl_extensions_count);
    SDL_Vulkan_GetInstanceExtensions(window, &sdl_extensions_count, sdl_extensions.data());

    // Combine extension lists
    instance_extension_names.insert(instance_extension_names.end(), sdl_extensions.begin(), sdl_extensions.end());

    // Create instance creation information
    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = nullptr;
    instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extension_names.size());
    instance_create_info.ppEnabledExtensionNames = instance_extension_names.data();
    instance_create_info.pNext = nullptr;
    if (use_validation_layers) {
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        instance_create_info.ppEnabledLayerNames = validation_layers.data();
    }

    // Create instance and assign to handle
    VK_CHECK(vkCreateInstance(&instance_create_info, nullptr, &instance));
    volkLoadInstance(instance);

    /* -------- Surface -------- */
    // Create the Vulkan surface
    SDL_Vulkan_CreateSurface(window, instance, &surface);
    if (surface == nullptr) {
        throw std::runtime_error("Failed to create surface!");
    }

    /* -------- Physical Device -------- */
    VkPhysicalDevice physical_device;

    // Get count of devices
    uint32_t device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());

    // First, just assign to the first device in the list. This makes sure that a device is
    // always selected even if it isn't a discrete GPU (next check)
    physical_device = physical_devices.front();

    // For all of our found devices
    for (const auto &device: physical_devices) {
        VkPhysicalDeviceProperties physical_device_properties;
        vkGetPhysicalDeviceProperties(device, &physical_device_properties);
        // Select if the physical device is a discrete GPU
        // For the time being don't check if the above features12 and features13 are supported, that can
        // be implemented later
        if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physical_device = device;
        }
    }

    // Assign handle
    selected_gpu = physical_device;

    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
    // https://github.com/zeux/volk?tab=readme-ov-file#optimizing-device-calls

    // Create queue family struct
    struct queue_family_indices_struct {
        // Graphics queue family
        std::optional<uint32_t> graphics_family;

        bool has_graphics_family() {
            return graphics_family.has_value();
        }

        // Compute queue family
        std::optional<uint32_t> compute_family;

        bool has_compute_family() {
            return compute_family.has_value();
        }

        // Present queue family
        std::optional<uint32_t> present_family;

        bool has_present_family() {
            return present_family.has_value();
        }
    };

    queue_family_indices_struct queue_family_indices;

    // Get vector of queue family property objects
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    // Add more of these loops to get indices for multiple families and throw error if they don't exist
    // Graphics queue
    for (int i = 0; i < queue_family_count; i++) {
        // Try to find graphics queue family via checking if reference exists in queueFlags bitmask
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family_indices.graphics_family = i;
            break;
        }
    }
    if (!queue_family_indices.has_graphics_family()) {
        throw std::runtime_error("Queue family supporting graphics not found!");
    }

    // Compute queue
    for (int i = 0; i < queue_family_count; i++) {
        // Try to find graphics queue family via checking if reference exists in queueFlags bitmask
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            queue_family_indices.compute_family = i;
            break;
        }
    }
    if (!queue_family_indices.has_compute_family()) {
        throw std::runtime_error("Queue family supporting compute not found!");
    }

    /* -------- Logical Device -------- */
    float graphics_queue_priority = 1.0;

    // Graphics queue
    VkDeviceQueueCreateInfo graphics_queue_create_info = {};
    graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_create_info.pNext = nullptr;
    graphics_queue_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
    graphics_queue_create_info.queueCount = 1;
    graphics_queue_create_info.pQueuePriorities = &graphics_queue_priority;

    // Enable some Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13 = {};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    // Enable some Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12 = {};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;
    // features12.pNext = &features13;

    // Create logical device features, links to future features struct chain
    VkPhysicalDeviceFeatures2 device_features = {};
    device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    device_features.pNext = &features12;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {};
    dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_features = {};
    synchronization2_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    synchronization2_features.synchronization2 = VK_TRUE;

    // Must manually add Vulan 1.3 features for MoltenVK compatibility (still not on version 1.3)
    std::vector<const char *> device_extension_names;
    device_extension_names.push_back("VK_KHR_swapchain");
    device_extension_names.push_back("VK_KHR_dynamic_rendering");
    // device_extension_names.push_back("VK_KHR_surface");
    device_extension_names.push_back("VK_KHR_portability_subset");
    device_extension_names.push_back("VK_KHR_synchronization2");

    // Make the creation information struct
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.pQueueCreateInfos = &graphics_queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pNext = &device_features;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extension_names.size());
    device_create_info.ppEnabledExtensionNames = device_extension_names.data();

    // Create the logical device
    VK_CHECK(vkCreateDevice(physical_device, &device_create_info, nullptr, &device));

    // Assign queue handle and family integer
    vkGetDeviceQueue(device, queue_family_indices.graphics_family.value(), 0, &graphics_queue);
    graphics_queue_family_index = queue_family_indices.graphics_family.value();
    if (graphics_queue == nullptr) {
        throw std::runtime_error("Failed to create graphics queue!");
    }

    // Command to reduce volk overhead
    volkLoadDevice(device);
}


void IncandescentEngine::initialize_swapchain(int width, int height) {
    // Struct to access hardware supports
    struct swapchain_support_details_struct {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    swapchain_support_details_struct swapchain_support_details = {};

    // Get basic capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selected_gpu, surface, &swapchain_support_details.surface_capabilities);

    // Get supported formats
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(selected_gpu, surface, &format_count, nullptr);

    if (format_count != 0) {
        swapchain_support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            selected_gpu, surface, &format_count, swapchain_support_details.formats.data());
    }

    // Get supported presentation modes
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(selected_gpu, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        swapchain_support_details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            selected_gpu, surface, &present_mode_count, swapchain_support_details.present_modes.data());
    }

    // Pick SRGB if possible, otherwise pick first
    swapchain_surface_format = swapchain_support_details.formats[0];
    for (const auto &available_format: swapchain_support_details.formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            swapchain_surface_format = available_format;
        }
    }

    // Choose present mode; prefer mailbox (triple buffering), then fifo (vsync)
    present_mode = swapchain_support_details.present_modes[0];
    for (const auto &available_present_mode: swapchain_support_details.present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = available_present_mode;
            break;
        }
        if (available_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
            present_mode = available_present_mode;
        }
    }

    // Set extent
    if (swapchain_support_details.surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapchain_extent = swapchain_support_details.surface_capabilities.currentExtent;
    } else {
        // Asks SDL for the actual pixel size in case we are using something like a Retina display that lies
        SDL_Vulkan_GetDrawableSize(window, &width, &height);

        // Assign SDL returned width and height
        swapchain_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        // Clamp to allowed extents
        swapchain_extent.width = std::clamp(swapchain_extent.width,
                                            swapchain_support_details.surface_capabilities.minImageExtent.width,
                                            swapchain_support_details.surface_capabilities.maxImageExtent.width);
        swapchain_extent.height = std::clamp(swapchain_extent.height,
                                             swapchain_support_details.surface_capabilities.minImageExtent.height,
                                             swapchain_support_details.surface_capabilities.maxImageExtent.height);
    }

    // Specify swapchain size
    uint32_t image_count = swapchain_support_details.surface_capabilities.minImageCount + 1;
    // Dont exceed maximum
    if (swapchain_support_details.surface_capabilities.maxImageCount > 0 &&
        image_count > swapchain_support_details.surface_capabilities.maxImageCount) {
        image_count = swapchain_support_details.surface_capabilities.maxImageCount;
    }

    // Create swapchain creation information
    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain_surface_format.format;
    swapchain_create_info.imageColorSpace = swapchain_surface_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageArrayLayers = 1; // 1 because we are not going to use stereoscopic 3d lol
    // This is really cool, basically what is happening is that both of these flags tell the corresponding bits
    // in the thing that gets passed and read by imageUsage to be 1. Because we are doing a bitwise OR, they are
    // both 1 because OR makes them 1 instead of 0 in the combined result if one or both are 1.
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = swapchain_support_details.surface_capabilities.currentTransform; // dont flip
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // dont blend window
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE; // change later when we need to handle recreating swapchain

    // Create swapchain
    VK_CHECK(vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain));

    // Create swapchain images
    uint32_t swapchain_image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
    swapchain_images.resize(swapchain_image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images.data());

    // Create swapchain image views
    swapchain_image_views.resize(swapchain_image_count);

    // For each image in the swapchain, create an image view
    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        // Fill creation information struct
        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.pNext = nullptr;
        image_view_create_info.image = swapchain_images[i];  // We are looping over images, so this picks what we're on
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain_surface_format.format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        // Create single image view
        VK_CHECK(vkCreateImageView(device, &image_view_create_info, nullptr, &swapchain_image_views[i]));
    }
}

void IncandescentEngine::initialize_commands() {
    // Create a command pool information struct for the graphics queue
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;
    // We need to allow resetting of individual command buffers
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = graphics_queue_family_index;

    // Create command pool and command buffer for each frame
    for (auto &[command_pool, main_command_buffer] : frames) {  // Each command_pool and main_command_buffer in frames
        VK_CHECK(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool));

        // Allocate command buffer
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.pNext = nullptr;
        command_buffer_allocate_info.commandPool = command_pool;
        command_buffer_allocate_info.commandBufferCount = 1;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  // Can be submitted directly
        VK_CHECK(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &main_command_buffer));
    }
}

void IncandescentEngine::initialize_sync_structures() {
    // To be implemented
}

void IncandescentEngine::cleanup() {
    if (is_initialized) {
        // We must destroy in the reverse order we created (newest first)
        destroy_swapchain();  // swapchain
        vkDestroySurfaceKHR(instance, surface, nullptr);  // surface
        vkDestroyDevice(device, nullptr);  // device
        vkDestroyInstance(instance, nullptr);  // instance
        SDL_DestroyWindow(window);  // window
    }

    // clear reference to now destroyed window/engine
    loaded_engine = nullptr;
}

void IncandescentEngine::destroy_swapchain() {
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    // Destroy swapchain images views
    for (uint32_t i = 0; i < swapchain_images.size(); i++) {
        vkDestroyImageView(device, swapchain_image_views[i], nullptr);
    }
}


void IncandescentEngine::draw() {
    // To be implemented
}

void IncandescentEngine::run() {
    SDL_Event event;
    bool quit = false;

    // While we haven't quit the window
    while (!quit) {
        // While there are still events in the queue

        // I think what this does is take the address of event and assigns it to whatever poll event
        // spits out (from the OS), we can then check what this event is and do actions
        while (SDL_PollEvent(&event) != 0) {
            // Close the window if the user wants the window closed
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            if (event.type == SDL_KEYDOWN) {
                fmt::print("keylog: {}\n", event.key.keysym.sym);
            }


            // Handle minimizing and re-opening the window
            if (event.type == SDL_WINDOWEVENT) {
                // Stop rendering if the window is minimized
                if (event.window.event == SDL_WINDOW_MINIMIZED) {
                    stop_rendering = true;
                }
                // Begin rendering on window restore
                if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
                    stop_rendering = false;
                }
            }
        }

        // Handle minimized window, stall drawing until restored
        if (stop_rendering) {
            // Throttle
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Finally draw frame
        draw();
    }
}
