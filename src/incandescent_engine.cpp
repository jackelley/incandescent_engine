//
// Created by Jack Kelley on 8/24/24.
//
#include <incandescent_types.h>
#include <incandescent_engine.h>
#include <incandescent_images.h>
#include <incan_struct_init.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

#include <fstream>
#include <iostream>
#include <queue>
#include <chrono>
#include <thread>

#ifdef __APPLE__
#ifndef VK_USE_PLATFORM_MACOS_MVK
#define VK_USE_PLATFORM_MACOS_MVK
#endif
#endif

constexpr bool use_validation_layers = true;
constexpr bool use_api_dump = false;
constexpr bool use_log_file = true;

IncandescentEngine *loaded_engine = nullptr;

IncandescentEngine &IncandescentEngine::Get() {
    return *loaded_engine;
}

void IncandescentEngine::initialize() {
    // Create logfile
    std::ofstream log_file;
    log_file.open("./src/initialization_log_file.txt");
    log_file << "Created log " << std::chrono::system_clock::now() << "\n";
    log_file.close();

    // Initialize volk
    VkResult volk_init = volkInitialize();
    if (volk_init != VK_SUCCESS) {
        fmt::print("Vulkan loader failed!");
    }
    if (use_log_file) {
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
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
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
        log_file << "Window initialized\n";
        log_file.close();
    }

    initialize_vulkan();
    if (use_log_file) {
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
        log_file << "Vulkan initialized\n";
        log_file.close();
    }

    initialize_swapchain(WIDTH, HEIGHT);
    if (use_log_file) {
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
        log_file << "Swapchain initialized\n";
        log_file.close();
    }

    initialize_commands();
    if (use_log_file) {
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
        log_file << "Command buffers initialized\n";
        log_file.close();
    }

    initialize_sync_structures();
    if (use_log_file) {
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
        log_file << "Synchronization structures initialized\n";
        log_file.close();
    }

    initialize_descriptors();
    if (use_log_file) {
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
        log_file << "Descriptors initialized\n";
        log_file.close();
    }

    initialize_pipelines();
    if (use_log_file) {
        log_file.open("./src/initialization_log_file.txt", std::ios_base::app);
        log_file << "Pipelines initialized\n";
        log_file.close();
    }

    // Set success check bool to true
    is_initialized = true;
}

void IncandescentEngine::initialize_vulkan() {
    /* -------- Instance -------- */
    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = nullptr;
    application_info.pApplicationName = "Incandescent v0.1";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pEngineName = "Incandescent v0.1";
    application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    // Activate extensions portability bit for MoltenVK (funny mac)
    std::vector<const char *> instance_extension_names = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
    };
    // instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    // instance_extension_names.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    // instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);


    // Activate layers
    std::vector<const char *> validation_layers;
    validation_layers.push_back("VK_LAYER_KHRONOS_validation");
    if (use_api_dump) {
        validation_layers.push_back("VK_LAYER_LUNARG_api_dump");
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
    instance_create_info.enabledExtensionCount = instance_extension_names.size();
    instance_create_info.ppEnabledExtensionNames = instance_extension_names.data();
    instance_create_info.pNext = nullptr;
    if (use_validation_layers) {
        instance_create_info.enabledLayerCount = validation_layers.size();
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

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {};
    dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering_feature.dynamicRendering = 1;
    dynamic_rendering_feature.pNext = nullptr;

    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_features = {};
    synchronization2_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    synchronization2_features.synchronization2 = 1;
    synchronization2_features.pNext = &dynamic_rendering_feature;

    // Enable some Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12 = {};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;
    features12.pNext = &synchronization2_features;

    // Create logical device features, links to future features struct chain
    VkPhysicalDeviceFeatures2 device_features = {};
    device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    device_features.pNext = &features12;

    // Must manually add Vulkan 1.3 features for MoltenVK compatibility (still not on version 1.3)
    std::vector<const char *> device_extension_names = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, // needed for making a swapchain
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, // dynamic rendering
        VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, // needed for mac
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, // synchronization2
        VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME // needed for vkCmdBlitImage2KHR because MoltenVK isn't on Vulkan 1.3
    };
    // device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    // device_extension_names.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    // device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    // device_extension_names.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

    // Make the creation information struct
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.pQueueCreateInfos = &graphics_queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pNext = &device_features;
    device_create_info.enabledExtensionCount = device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = device_extension_names.data();
    device_create_info.pEnabledFeatures = nullptr;

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

    // We are using Volk, so we need to provide the memory related function pointers directly to VMA
    VmaVulkanFunctions vma_vulkan_functions{};
    vma_vulkan_functions.vkAllocateMemory = vkAllocateMemory;
    vma_vulkan_functions.vkBindBufferMemory = vkBindBufferMemory;
    vma_vulkan_functions.vkBindImageMemory = vkBindImageMemory;
    vma_vulkan_functions.vkCreateBuffer = vkCreateBuffer;
    vma_vulkan_functions.vkCreateImage = vkCreateImage;
    vma_vulkan_functions.vkDestroyBuffer = vkDestroyBuffer;
    vma_vulkan_functions.vkDestroyImage = vkDestroyImage;
    vma_vulkan_functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vma_vulkan_functions.vkFreeMemory = vkFreeMemory;
    vma_vulkan_functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vma_vulkan_functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vma_vulkan_functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vma_vulkan_functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vma_vulkan_functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vma_vulkan_functions.vkMapMemory = vkMapMemory;
    vma_vulkan_functions.vkUnmapMemory = vkUnmapMemory;
    vma_vulkan_functions.vkCmdCopyBuffer = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.physicalDevice = physical_device;
    allocator_create_info.device = device;
    allocator_create_info.instance = instance;
    allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT; // Lets us use GPU pointers
    allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;

    vmaCreateAllocator(&allocator_create_info, &allocator);
}


void IncandescentEngine::initialize_swapchain(int width, int height) {
    // Struct to access hardware supported capabilities
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
        image_view_create_info.image = swapchain_images[i]; // We are looping over images, so this picks what we're on
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

    /* -------- Create image and image view we will draw to -------- */

    VkExtent3D draw_image_extent = {WIDTH, HEIGHT, 1};

    // Hardcode draw format to 32-bit float
    draw_image.image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
    draw_image.image_extent = draw_image_extent;

    VkImageUsageFlags draw_image_usage_flags = {};
    // Read/write, usage_storage allows us to use compute shaders, color so we can do graphics
    draw_image_usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo image_create_info = incan_struct_init::image_create_info(
        draw_image.image_format, draw_image_usage_flags, draw_image_extent);

    VmaAllocationCreateInfo image_allocation_create_info = {};
    image_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY; // Tells VMA to put image into VRAM
    image_allocation_create_info.requiredFlags = static_cast<VkMemoryPropertyFlags>(
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // This guarantees fastest memory access

    // Allocate and create the image
    vmaCreateImage(allocator, &image_create_info, &image_allocation_create_info, &draw_image.image,
                   &draw_image.allocation, nullptr);

    VkImageViewCreateInfo image_view_create_info = incan_struct_init::image_view_create_info(
        draw_image.image_format, draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(device, &image_view_create_info, nullptr, &draw_image.image_view));
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
    for (auto &[command_pool, main_command_buffer, swapchain_semaphore, render_semaphore, render_fence]: frames) {
        VK_CHECK(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool));

        // Allocate command buffer
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.pNext = nullptr;
        command_buffer_allocate_info.commandPool = command_pool;
        command_buffer_allocate_info.commandBufferCount = 1;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Can be submitted directly
        VK_CHECK(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &main_command_buffer));
    }
}

void IncandescentEngine::initialize_sync_structures() {
    // Fence controls when the GPU finishes rendering the frame
    // Semaphores synchronize with the swapchain
    // Fence starts signaled so we can wait on it starting from the first frame
    VkFenceCreateInfo fence_create_info = incan_struct_init::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

    VkSemaphoreCreateInfo semaphore_create_info = incan_struct_init::semaphore_create_info();

    // Create fence and semaphores for each frame
    for (auto &[command_pool, main_command_buffer, swapchain_semaphore, render_semaphore, render_fence]: frames) {
        VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &render_fence));
        VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &swapchain_semaphore));
        VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &render_semaphore));
    }
}

void IncandescentEngine::cleanup() {
    if (is_initialized) {
        // We must destroy in the reverse order we created (newest first)
        if (is_initialized) {
            // Wait until the GPU completes all outstanding queue operations
            vkDeviceWaitIdle(device);

            for (auto &[command_pool, main_command_buffer, swapchain_semaphore,
                     render_semaphore, render_fence]: frames) {
                // Destroy command pool and buffers
                vkDestroyCommandPool(device, command_pool, nullptr);

                // Destroy sync objects
                vkDestroySemaphore(device, swapchain_semaphore, nullptr);
                vkDestroySemaphore(device, render_semaphore, nullptr);
                vkDestroyFence(device, render_fence, nullptr);
            }
        }
        // Flush global objects
        // vkDestroyShaderModule();
        vkDestroyImageView(device, draw_image.image_view, nullptr);
        vmaDestroyImage(allocator, draw_image.image, draw_image.allocation);
        vkDestroyPipelineLayout(device, gradient_pipeline_layout, nullptr);
        vkDestroyPipeline(device, gradient_pipeline, nullptr);
        global_descriptor_allocator.destroy_pool(device);
        vkDestroyDescriptorSetLayout(device, draw_image_descriptor_set_layout, nullptr);
        destroy_swapchain(); // swapchain
        vkDestroySurfaceKHR(instance, surface, nullptr); // surface
        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr); // device
        vkDestroyInstance(instance, nullptr); // instance
        SDL_DestroyWindow(window); // window
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

void IncandescentEngine::initialize_descriptors() {
    // Create a descriptor pool to hold 10 sets with 1 image each
    std::vector<DescriptorAllocator::PoolSizeRatio> pool_size_ratios = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

    global_descriptor_allocator.initialize_pool(device, 10, pool_size_ratios);

    // Create descriptor set layout for compute draw
    DescriptorLayoutBuilder descriptor_layout_builder;
    descriptor_layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    draw_image_descriptor_set_layout = descriptor_layout_builder.build(device, VK_SHADER_STAGE_COMPUTE_BIT);

    // Allocate descriptor set for the draw image
    draw_image_descriptor_set = global_descriptor_allocator.allocate(device, draw_image_descriptor_set_layout);

    VkDescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptor_image_info.imageView = draw_image.image_view;

    VkWriteDescriptorSet draw_image_write_descriptor_set = {};
    draw_image_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    draw_image_write_descriptor_set.pNext = nullptr;
    draw_image_write_descriptor_set.dstBinding = 0;
    draw_image_write_descriptor_set.dstSet = draw_image_descriptor_set;
    draw_image_write_descriptor_set.descriptorCount = 1;
    draw_image_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    draw_image_write_descriptor_set.pImageInfo = &descriptor_image_info;

    vkUpdateDescriptorSets(device, 1, &draw_image_write_descriptor_set, 0, nullptr);
}


void IncandescentEngine::initialize_pipelines() {
    initialize_background_pipelines();
}


void IncandescentEngine::initialize_background_pipelines() {
    // Compute pipeline
    VkPipelineLayoutCreateInfo compute_layout_create_info = {};
    compute_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    compute_layout_create_info.pNext = nullptr;
    compute_layout_create_info.pSetLayouts = &draw_image_descriptor_set_layout;
    compute_layout_create_info.setLayoutCount = 1;

    VK_CHECK(vkCreatePipelineLayout(device, &compute_layout_create_info, nullptr, &gradient_pipeline_layout));

    // Load shader
    VkShaderModule compute_draw_shader;
    if (!incan_util::load_shader_module("shaders/gradient.comp.spv", device, &compute_draw_shader)) {
        fmt::print("Error when building compute shader\n");
    }

    VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
    shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info.pNext = nullptr;
    shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage_create_info.module = compute_draw_shader;
    shader_stage_create_info.pName = "main";

    VkComputePipelineCreateInfo compute_pipeline_create_info = {};
    compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    compute_pipeline_create_info.pNext = nullptr;
    compute_pipeline_create_info.layout = gradient_pipeline_layout;
    compute_pipeline_create_info.stage = shader_stage_create_info;
    // compute_pipeline_create_info.basePipelineIndex = 0;

    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &gradient_pipeline));

    vkDestroyShaderModule(device, compute_draw_shader, nullptr);
}


void IncandescentEngine::draw() {
    // Start by waiting for the GPU to finish rendering the last frame, with a timeout of 1 second (nanoseconds)
    VK_CHECK(vkWaitForFences(device, 1, &get_current_frame().render_fence, true, 1000000000));
    VK_CHECK(vkResetFences(device, 1, &get_current_frame().render_fence)); // Reset the fence after use

    // Request image from swapchain, swapchain semaphore signals when image is acquired
    uint32_t swapchain_image_index;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().swapchain_semaphore,
        nullptr, &swapchain_image_index));

    // Get the command buffer for this frame
    VkCommandBuffer command_buffer = get_current_frame().main_command_buffer;

    // We know the commands are done executing because of the fence above, so we can safely reset the command buffer
    VK_CHECK(vkResetCommandBuffer(command_buffer, 0));

    // Get new command buffer begin info so we can start writing to the command buffer again
    // One time usage bit gives small speedup, we tell Vulkan we are only submitting and executing this buffer once
    VkCommandBufferBeginInfo command_buffer_begin_info =
            incan_struct_init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // Set the extent of the current image to the window size
    draw_extent.width = draw_image.image_extent.width;
    draw_extent.height = draw_image.image_extent.height;

    // Start writing to the command buffer
    VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

    // Transition the draw image into the general layout so we can write into it, the initial layout doesn't matter
    // as we are overwriting it.
    incan_util::transition_image_graphics_to_graphics(command_buffer, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED,
                                                      VK_IMAGE_LAYOUT_GENERAL);
    // Call draw command
    draw_background(command_buffer);

    // Transition draw image to transfer source
    incan_util::transition_image_graphics_to_graphics(command_buffer, draw_image.image, VK_IMAGE_LAYOUT_GENERAL,
                                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    // Transition swapchain image to transfer destination
    incan_util::transition_image_graphics_to_graphics(command_buffer, swapchain_images[swapchain_image_index],
                                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy draw image to swapchain
    incan_util::copy_image_to_image(command_buffer, draw_image.image, swapchain_images[swapchain_image_index],
                                    draw_extent, swapchain_extent);

    // Set swapchain image layout to present
    incan_util::transition_image_graphics_to_graphics(command_buffer, swapchain_images[swapchain_image_index],
                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Finalize command buffer
    VK_CHECK(vkEndCommandBuffer(command_buffer));

    // Prepare the queue submission
    VkCommandBufferSubmitInfo command_buffer_submit_info =
            incan_struct_init::command_buffer_submit_info(command_buffer);

    // We want to wait on the semaphore that is signalled when the swapchain is ready
    VkSemaphoreSubmitInfo wait_info =
            incan_struct_init::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                                                     get_current_frame().swapchain_semaphore);

    // We signal when the rendering is done with this semaphore
    VkSemaphoreSubmitInfo signal_info =
            incan_struct_init::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                                                     get_current_frame().render_semaphore);

    VkSubmitInfo2 submit_info = incan_struct_init::submit_info(&command_buffer_submit_info, &signal_info, &wait_info);

    // Submit the command buffer, render_fence will block the queue until this command buffer finishes
    VK_CHECK(vkQueueSubmit2KHR(graphics_queue, 1, &submit_info, get_current_frame().render_fence));

    // Present the rendered image to the window, we will wait on the render semaphore
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.pSwapchains = &swapchain;
    present_info.swapchainCount = 1;
    present_info.pWaitSemaphores = &get_current_frame().render_semaphore;
    present_info.waitSemaphoreCount = 1;
    present_info.pImageIndices = &swapchain_image_index;

    VK_CHECK(vkQueuePresentKHR(graphics_queue, &present_info));

    // Increment frame number
    frame_number++;
}

void IncandescentEngine::draw_background(VkCommandBuffer command_buffer) {
    // Make a clear-color based off the frame number, repeating over 120 frames
    VkClearColorValue clear_color_value;
    float flash = std::abs(std::sin(frame_number / 120.f));
    clear_color_value = {{0.0f, 0.0f, flash, 1.0f}};

    VkImageSubresourceRange clear_color_range = incan_struct_init::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    // Bind the gradient draw compute pipeline
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline);

    // Bind descriptor set containing draw image for the compute pipeline
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_layout, 0, 1,
                            &draw_image_descriptor_set, 0, nullptr);

    // Dispatch pipeline, must match our compute shader workgroup size
    vkCmdDispatch(command_buffer, std::ceil(draw_extent.width / 16.0), std::ceil(draw_extent.height / 16.0), 1);
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
