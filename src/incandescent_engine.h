//
// Created by Jack Kelley on 8/24/24.
//

#ifndef INCANDESCENT_ENGINE_H
#define INCANDESCENT_ENGINE_H

#include "incandescent_types.h"

// Create frame data struct
struct FrameData {
    // If adding more command buffer in the future, either make main_command buffer a vector and change logic
    // in initialize_commands, or create a new vector and change logic as well.
    VkCommandPool command_pool;
    VkCommandBuffer main_command_buffer;
    // Synchronization structures
    VkSemaphore swapchain_semaphore; // Lets the render commands wait on the swapchain image request
    VkSemaphore render_semaphore; // Controls presenting the image once the draw is finished
    VkFence render_fence; // Lets us wait for the draw commands for the frame to be finished
};

constexpr unsigned int FRAME_OVERLAP = 2;

class IncandescentEngine {
public:
    // Frame information
    FrameData frames[FRAME_OVERLAP];
    // Gets the address of the current frame, allows us to not worry about directly accessing the frames array
    FrameData &get_current_frame() {
        return frames[frame_number % FRAME_OVERLAP];
    }

    // Internal flags
    bool is_initialized = false;
    int frame_number = 0;
    bool stop_rendering = false;
    const int WIDTH = 1920;
    const int HEIGHT = 1080;

    // Vulkan instance
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice selected_gpu;
    VkDevice device;
    VkSurfaceKHR surface;

    // Vulkan swapchain
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR swapchain_surface_format;
    VkPresentModeKHR present_mode;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    VkExtent2D swapchain_extent;

    // Vulkan queue
    VkQueue graphics_queue;
    uint32_t graphics_queue_family_index;

    // Forward declaration reduces compile times and ambiguity for the compiler
    struct SDL_Window *window = nullptr;

    // Ensures that there is only one instance of the engine somehow
    static IncandescentEngine &Get();

    // Initializes the entire engine
    void initialize();

    // Initializes Vulkan context
    void initialize_vulkan();

    // Initializes the swapchain
    void initialize_swapchain(int width, int height);

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

private:
    void destroy_swapchain();
};

#endif //INCANDESCENT_ENGINE_H
