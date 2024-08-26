//
// Created by Jack Kelley on 8/24/24.
//

#ifndef INCANDESCENT_ENGINE_H
#define INCANDESCENT_ENGINE_H

#include "incandescent_types.h"
#include "VkBootstrapDispatch.h"

class IncandescentEngine {
public:

    // Internal flags
    bool is_initialized = false;
    int frame_number = 0;
    bool stop_rendering = false;
    const int WIDTH = 1700;
    const int HEIGHT = 900;

    // Vulkan instance
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice selected_gpu;
    VkDevice device;
    VkSurfaceKHR surface;

    // Forward declaration reduces compile times and ambiguity for the compiler
    struct SDL_Window* window = nullptr;

    // Ensures that there is only one instance of the engine somehow
    static IncandescentEngine& Get();

    // Initializes the entire engine
    void initialize();

    // Initializes Vulkan context
    void initialize_vulkan();

    // Initializes the swapchain
    void initialize_swapchain();

    // Initializes the command system
    void initialize_commands();

    // Initializes the sync structures
    void initialize_sync_structures();

    // Shuts down the engine and cleans memory
    void cleanup();

    // Contains the draw loop
    void draw();

    // Runs the main program loop
    void run();
};


#endif //INCANDESCENT_ENGINE_H
