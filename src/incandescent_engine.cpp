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
#include <iostream>
#include <queue>
#include <volk.h>

constexpr bool use_validation_layers = true;

IncandescentEngine* loaded_engine = nullptr;

IncandescentEngine& IncandescentEngine::Get() {
    return *loaded_engine;
}

void IncandescentEngine::initialize() {
    // Initialize volk
    VkResult volk_init = volkInitialize();
    if (volk_init != VK_SUCCESS) {
        fmt::print("Vulkan loader failed!");
    }

    // Make sure that there isn't already an initialized engine
    assert(loaded_engine == nullptr);
    loaded_engine = this;

    // Create the window
    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = (SDL_WINDOW_VULKAN);

    window = SDL_CreateWindow("Incandescent Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WIDTH, HEIGHT, window_flags);

    initialize_vulkan();

    initialize_swapchain();

    initialize_commands();

    initialize_sync_structures();

    // Set success check bool to true
    is_initialized = true;
}

void IncandescentEngine::initialize_vulkan() {
    /* -------- Instance -------- */

    // Get number of extensions
    u_int32_t extension_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);

    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = "Incandescent v0.1";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    // Activate portability bit for MoltenVK (funny mac)
    std::vector<const char*> extension_names;
    extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extension_names.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extension_count += 2;

    // Get SDL needed extensions
    std::vector<const char*> sdl_extensions;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, sdl_extensions.data());

    // Combine extension lists
    extension_names.insert(extension_names.end(), sdl_extensions.begin(), sdl_extensions.end());

    // Create instance creation information
    VkInstanceCreateInfo inst_info = {};
    inst_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount = static_cast<uint32_t>(extension_names.size());
    inst_info.ppEnabledExtensionNames = extension_names.data();

    // Create instance and assign to handle
    VkResult instance_result = vkCreateInstance(&inst_info, nullptr, &instance);
    volkLoadInstance(instance);

    // Validation
    if (instance_result < 0) {
        std::cout << "Failed to create instance!" << std::endl;
    }

    /* -------- Surface -------- */
    // Create the Vulkan surface
    SDL_Vulkan_CreateSurface(window, instance, &surface);

    /* -------- Physical Device -------- */
    VkPhysicalDevice physical_device;

    // Get count of devices
    u_int32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) {
        throw std::runtime_error("Failed to find  GPU!");
    }

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());

    // First, just assign to the first device in the list. This makes sure that a device is
    // always selected even if it isn't a discrete GPU (next check)
    physical_device = physical_devices.front();

    // For all of our found devices
    for (const auto &device : physical_devices) {
        VkPhysicalDeviceProperties physical_device_properties;
        vkGetPhysicalDeviceProperties(device, &physical_device_properties);
        // Select if the physical device is a discrete GPU
        // For the time being don't check if the above features12 and features13 are supported, that can
        // be implemented later
        if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physical_device = device;
        }
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
    // https://github.com/zeux/volk?tab=readme-ov-file#optimizing-device-calls

    // Create queue family struct for later
    struct queue_family_indices_struct {
        // Graphics queue family
        std::optional<u_int32_t> graphics_family;
        // Add to this to check for other graphics families
        bool has_graphics_family() {
            return graphics_family.has_value();
        }
    };

    queue_family_indices_struct queue_family_indices;

    // Get vector of queue family property objects
    u_int32_t queue_family_count = 0;
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

    // Compute queue
    if (!queue_family_indices.has_graphics_family()) {
        throw std::runtime_error("Queue family supporting graphics not found!");
    }
    for (int i = 0; i < queue_family_count; i++) {
        // Try to find graphics queue family via checking if reference exists in queueFlags bitmask
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            queue_family_indices.graphics_family = i;
            break;
        }
    }
    if (!queue_family_indices.has_graphics_family()) {
        throw std::runtime_error("Queue family supporting compute not found!");
    }

    /* -------- Logical Device -------- */
    float graphics_queue_priority = 1.0;

    // Graphics queue
    VkDeviceQueueCreateInfo graphics_queue_create_info{};
    graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
    graphics_queue_create_info.queueCount = 1;
    graphics_queue_create_info.pQueuePriorities = &graphics_queue_priority;

    // Enable some Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13;
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    // Enable some Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12;
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;
    features13.pNext = &features13;

    // Create logical device features, links to future features struct chain
    VkPhysicalDeviceFeatures2 device_features = {};
    device_features.pNext = &features12;

    // Make the creation information struct
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &graphics_queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pNext = &device_features;

    // Create the logical device
    if(vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    // Command to reduce volk overhead
    volkLoadDevice(device);
}


void IncandescentEngine::initialize_swapchain() {
    // To be implemented
}

void IncandescentEngine::initialize_commands() {
    // To be implemented
}

void IncandescentEngine::initialize_sync_structures() {
    // To be implemented
}




void IncandescentEngine::cleanup() {
    if (is_initialized) {
        SDL_DestroyWindow(window);
    }

    // clear reference to now destroyed window/engine
    loaded_engine = nullptr;
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
