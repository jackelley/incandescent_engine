//
// Created by Jack Kelley on 9/1/24.
//

#include <fstream>
#include <incan_struct_init.h>
#include <incandescent_types.h>

VkImageSubresourceRange incan_struct_init::image_subresource_range(VkImageAspectFlags aspect_flags) {
    // Range specifying the entire image
    VkImageSubresourceRange image_subresource_range = {};
    image_subresource_range.aspectMask = aspect_flags;
    image_subresource_range.baseMipLevel = 0;
    image_subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
    image_subresource_range.baseArrayLayer = 0;
    image_subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return image_subresource_range;
}

VkFenceCreateInfo incan_struct_init::fence_create_info(VkFenceCreateFlags flags) {
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = nullptr;
    fence_create_info.flags = flags;

    return fence_create_info;
}

VkSemaphoreCreateInfo incan_struct_init::semaphore_create_info(VkSemaphoreCreateFlags flags) {
    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = nullptr;
    semaphore_create_info.flags = flags;

    return semaphore_create_info;
}

VkSemaphoreSubmitInfo
incan_struct_init::semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore) {
    VkSemaphoreSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.semaphore = semaphore;
    submit_info.stageMask = stage_mask;
    submit_info.deviceIndex = 0;
    submit_info.value = 1;

    return submit_info;
}

VkCommandBufferBeginInfo incan_struct_init::command_buffer_begin_info(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.pInheritanceInfo = nullptr;
    command_buffer_begin_info.flags = flags;

    return command_buffer_begin_info;
}

VkCommandBufferSubmitInfo incan_struct_init::command_buffer_submit_info(VkCommandBuffer command_buffer) {
    VkCommandBufferSubmitInfo command_buffer_submit_info = {};
    command_buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    command_buffer_submit_info.pNext = nullptr;
    command_buffer_submit_info.commandBuffer = command_buffer;
    command_buffer_submit_info.deviceMask = 0;

    return command_buffer_submit_info;
}

VkSubmitInfo2 incan_struct_init::submit_info(VkCommandBufferSubmitInfo *command_buffer_submit_info,
                                             VkSemaphoreSubmitInfo *signal_semaphore_submit_info,
                                             VkSemaphoreSubmitInfo *wait_semaphore_submit_info) {
    VkSubmitInfo2 submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreInfoCount = wait_semaphore_submit_info == nullptr ? 0 : 1;
    submit_info.pWaitSemaphoreInfos = wait_semaphore_submit_info;
    submit_info.signalSemaphoreInfoCount = signal_semaphore_submit_info == nullptr ? 0 : 1;
    submit_info.pSignalSemaphoreInfos = signal_semaphore_submit_info;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = command_buffer_submit_info;

    return submit_info;
}

VkImageCreateInfo incan_struct_init::image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent) {
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent = extent;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    // Used for MSAA, we will not use it by default
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    // Store image on best GPU format
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = usage_flags;

    return image_create_info;
}

VkImageViewCreateInfo incan_struct_init::image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags) {
    // Build an image view for the depth image we will use for rendering
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = nullptr;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.image = image;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.aspectMask = aspect_flags;

    return image_view_create_info;
}