//
// Created by Jack Kelley on 9/1/24.
//

#include <incandescent_images.h>
#include <volk.h>
#include <incan_struct_init.h>
#define VOLK_IMPLEMENTATION

/*
 * TODO - This needs to be updated later to not use the all commands bit, and instead be several functions that are
 * more specific and light weight for each transition we need
 */
void incan_util::transition_image_graphics_graphics(VkCommandBuffer command_buffer, VkImage image,
                                                    VkImageLayout current_layout,
                                                    VkImageLayout new_layout) {
    VkImageMemoryBarrier2 image_barrier = {};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    image_barrier.pNext = nullptr;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    image_barrier.oldLayout = current_layout;
    image_barrier.newLayout = new_layout;

    // Subresource range lets us target only one part of an image with the barrier, right now this does all levels
    VkImageAspectFlags aspect_flags = new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                                          ? VK_IMAGE_ASPECT_DEPTH_BIT
                                          : VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageSubresourceRange sub_image = incan_struct_init::image_subresource_range(aspect_flags);

    image_barrier.subresourceRange = sub_image;
    image_barrier.image = image;

    VkDependencyInfo dependency_info = {};
    dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependency_info.pNext = nullptr;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &image_barrier;

    vkCmdPipelineBarrier2KHR(command_buffer, &dependency_info);
}
