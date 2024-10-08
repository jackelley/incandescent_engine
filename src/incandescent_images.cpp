//
// Created by Jack Kelley on 9/1/24.
//

#include <incandescent_images.h>
#include <volk.h>
#include <incan_struct_init.h>
#include <fstream>

/*
 * TODO - This needs to be updated later to not use the all commands bit, and instead be several functions that are
 * more specific and light weight for each transition we need
 * https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
 */
void incan_util::transition_image_graphics_to_graphics(VkCommandBuffer command_buffer, VkImage image,
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

void incan_util::copy_image_to_image(VkCommandBuffer command_buffer, VkImage source, VkImage destination,
                                     VkExtent2D source_extent, VkExtent2D destination_extent) {
    // Specify region to blit (whole image for both)
    VkImageBlit2KHR blit_region = {};
    blit_region.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR;
    blit_region.pNext = nullptr;
    blit_region.srcOffsets[1].x = source_extent.width;
    blit_region.srcOffsets[1].y = source_extent.height;
    blit_region.srcOffsets[1].z = 1;
    blit_region.dstOffsets[1].x = destination_extent.width;
    blit_region.dstOffsets[1].y = destination_extent.height;
    blit_region.dstOffsets[1].z = 1;
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;

    // Specify formats and source/destination
    VkBlitImageInfo2KHR blit_image_info = {};
    blit_image_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR;
    blit_image_info.pNext = nullptr;
    blit_image_info.srcImage = source;
    blit_image_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blit_image_info.dstImage = destination;
    blit_image_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blit_image_info.filter = VK_FILTER_LINEAR;
    blit_image_info.regionCount = 1;
    blit_image_info.pRegions = &blit_region;

    // Execute blit command
    vkCmdBlitImage2KHR(command_buffer, &blit_image_info);
}