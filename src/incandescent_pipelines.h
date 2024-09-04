//
// Created by Jack Kelley on 9/2/24.
//

#ifndef INCANDESCENT_PIPELINES_H
#define INCANDESCENT_PIPELINES_H

#include "incandescent_pipelines.h"
#include <fstream>
#include <incan_struct_init.h>

namespace incan_util {
    bool load_shader_module(const char *file_path, VkDevice device, VkShaderModule *out_shader_module);
}

class incandescent_pipelines {

};



#endif //INCANDESCENT_PIPELINES_H
