//
// Created by Jack Kelley on 8/24/24.
//

#ifndef INCANDESCENT_ENGINE_H
#define INCANDESCENT_ENGINE_H

#include "incandescent_types.h"

class IncandescentEngine {
public:
    bool is_initialized = false;
    int frame_number = 0;
    bool stop_rendering = false;
    VkExtent2D window_extent{1700, 900};

    // Forward declaration reduces compile times and ambiguity for the compiler
    struct SDL_Window *window = nullptr;

    // Ensures that there is only one instance of the engine somehow
    static IncandescentEngine &Get();

    // Initializes the entire engine
    void initialize();

    // Shuts down the engine and cleans memory
    void cleanup();

    // Contains the draw loop
    void draw();

    // Runs the main program loop
    void run();
};


#endif //INCANDESCENT_ENGINE_H
