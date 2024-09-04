//
// Created by Jack Kelley on 9/2/24.
//

#include "incandescent_pipelines.h"
#include <fstream>
#include <incan_struct_init.h>

bool incan_util::load_shader_module(const char *file_path, VkDevice device, VkShaderModule *out_shader_module) {
    // Open file with cursor at the end
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    // Find the file size by looking at cursor location
    size_t file_size = file.tellg();

    // SPIR-V expects uint32_t buffer
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    // Cursor to start
    file.seekg(0);

    // Load file into buffer
    file.read(reinterpret_cast<char *>(buffer.data()), file_size);

    // Close file after reading
    file.close();

    // Create new shader module
    VkShaderModuleCreateInfo shader_module_create_info = {};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.pNext = nullptr;
    // Multiply size by type size to get buffer size in bytes
    shader_module_create_info.codeSize = buffer.size() * sizeof(uint32_t);
    shader_module_create_info.pCode = buffer.data();

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &shader_module_create_info, nullptr, &shader_module) != VK_SUCCESS) {
        return false;
    }

    *out_shader_module = shader_module;
    return true;
}


