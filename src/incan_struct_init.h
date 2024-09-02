//
// Created by Jack Kelley on 9/1/24.
//

#ifndef INCAN_STRUCT_INIT_H
#define INCAN_STRUCT_INIT_H
#include <incandescent_images.h>
#include <incandescent_types.h>
/*
 * Contains small functions to simplify making create info structs
 */
namespace incan_struct_init {
    VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspect_flags = 0);

    VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);

    VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

    VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore);

    VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

    VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer command_buffer);

    VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo *command_buffer_submit_info,
                              VkSemaphoreSubmitInfo *signal_semaphore_submit_info,
                              VkSemaphoreSubmitInfo *wait_semaphore_submit_info);
}
#endif //INCAN_STRUCT_INIT_H
