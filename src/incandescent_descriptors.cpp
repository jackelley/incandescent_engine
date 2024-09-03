//
// Created by Jack Kelley on 9/2/24.
//

#include <incandescent_types.h>
#include <incandescent_descriptors.h>
#include <volk.h>


void DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type) {
    VkDescriptorSetLayoutBinding new_binding = {};
    new_binding.binding = binding;
    new_binding.descriptorCount = 1;
    new_binding.descriptorType = type;

    bindings.push_back(new_binding);
}

void DescriptorLayoutBuilder::clear() {
    bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shader_stages, void *pNext,
                                                     VkDescriptorSetLayoutCreateFlags flags) {
    // Add stage flags for each binding
    for (auto &binding: bindings) {
        binding.stageFlags |= shader_stages;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pNext = pNext;
    descriptor_set_layout_create_info.pBindings = bindings.data();
    descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_layout_create_info.flags = flags;

    VkDescriptorSetLayout set;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info, nullptr, &set));

    return set;
}

void DescriptorAllocator::initialize_pool(VkDevice device, uint32_t max_sets,
                                          std::span<PoolSizeRatio> pool_size_ratios) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (PoolSizeRatio pool_size_ratio: pool_size_ratios) {
        pool_sizes.push_back(VkDescriptorPoolSize{
            .type = pool_size_ratio.type, .descriptorCount = static_cast<uint32_t>(pool_size_ratio.ratio * max_sets)
        });
    }

    VkDescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = 0;
    pool_create_info.maxSets = max_sets;
    pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();

    vkCreateDescriptorPool(device, &pool_create_info, nullptr, &pool);
}

void DescriptorAllocator::clear_descriptors(VkDevice device) {
    vkResetDescriptorPool(device, pool, 0);
}

void DescriptorAllocator::destroy_pool(VkDevice device) {
    vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = nullptr;
    allocate_info.descriptorPool = pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &layout;

    VkDescriptorSet descriptor_set;
    VK_CHECK(vkAllocateDescriptorSets(device, &allocate_info, &descriptor_set));

    return descriptor_set;
}
