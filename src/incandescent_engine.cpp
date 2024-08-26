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

    // Validation
    if (instance_result < 0) {
        std::cout << "Failed to create instance!" << std::endl;
    }

    /* -------- Surface -------- */
    // Create the Vulkan surface
    SDL_Vulkan_CreateSurface(window, instance, &surface);

    /* -------- Device -------- */
    // Enable some Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13;
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    // Enable some Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12;
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    VkPhysicalDevice physical_device;

    // Get count of devices
    u_int32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) {
        throw std::runtime_error("Failed to find supported GPU!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    // ??? NEXT STEP IS TO CHECK WHETHER DEVICES ARE SUITABLE ???
    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
    // https://github.com/zeux/volk?tab=readme-ov-file#optimizing-device-calls
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
