//
// Created by Jack Kelley on 9/2/24.
//


#ifndef INCANDESCENT_DESCRIPTORS_H
#define INCANDESCENT_DESCRIPTORS_H
#include <vector>
#include <vulkan/vulkan_core.h>
#include <incandescent_types.h>

struct DescriptorLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    void add_binding(uint32_t binding, VkDescriptorType type);

    void clear();

    VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shader_stages, void *pNext = nullptr,
                                VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator {
    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

    VkDescriptorPool pool;
    void initialize_pool(VkDevice device, uint32_t max_sets, std::span<PoolSizeRatio> pool_size_ratios);
    void clear_descriptors(VkDevice device);
    void destroy_pool(VkDevice device);
    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

class incandescent_descriptors {
};


#endif //INCANDESCENT_DESCRIPTORS_H
