//
// Created by Jack Kelley on 9/1/24.
//

#ifndef INCANDESCENT_IMAGES_H
#define INCANDESCENT_IMAGES_H

#include <incandescent_types.h>
#include <incandescent_engine.h>

namespace incan_util {
    void transition_image_graphics_to_graphics(VkCommandBuffer command_buffer, VkImage image,
                                               VkImageLayout current_layout,
                                               VkImageLayout new_layout);

    void copy_image_to_image(VkCommandBuffer command_buffer, VkImage source, VkImage destination,
                             VkExtent2D source_extent, VkExtent2D destination_extent);

    bool load_shader_module(const char *file_path, VkDevice device, VkShaderModule *out_shader_module);
}


class incandescent_images {
};


#endif //INCANDESCENT_IMAGES_H
