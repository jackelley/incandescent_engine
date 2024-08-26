//
// Created by Jack Kelley on 8/24/24.
//
#include <incandescent_types.h>
#include <incandescent_engine.h>

#include <chrono>
#include <thread>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

constexpr bool use_validation_layers = true;

IncandescentEngine* loaded_engine = nullptr;

IncandescentEngine& IncandescentEngine::Get() {
    return *loaded_engine;
}

void IncandescentEngine::initialize() {
    // Set up dynamic function dispatcher

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
    vk::DynamicLoader dynamic_loader;
    auto vkGetInstanceProcAddr = dynamic_loader.getProcAddress<PFN_vkGetInstanceProcAddr>(
        "vkGetInstanceProcAddr");

    /* -------- Instance -------- */

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
