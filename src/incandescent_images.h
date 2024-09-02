//
// Created by Jack Kelley on 9/1/24.
//

#ifndef INCANDESCENT_IMAGES_H
#define INCANDESCENT_IMAGES_H

#include <incandescent_types.h>
#include <incandescent_engine.h>

namespace incan_util {
    void transition_image_graphics_graphics(VkCommandBuffer command_buffer, VkImage image, VkImageLayout current_layout,
                                            VkImageLayout new_layout);
}


class incandescent_images {
};


#endif //INCANDESCENT_IMAGES_H
