cmake_minimum_required(VERSION 3.29)
project(incandescent-v0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan COMPONENTS MoltenVK)
find_package(Vulkan)
find_package(Eigen3 3.4 REQUIRED NO_MODULE)
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(fmt CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY src)

add_executable(incandescent-v0.1 src/main.cpp
        src/incandescent_engine.cpp
        src/incandescent_engine.h
        src/incandescent_types.h
        src/incandescent_images.cpp
        src/incandescent_images.h
        src/incan_struct_init.cpp
        src/incan_struct_init.h
)

if (APPLE)
    target_compile_definitions(incandescent-v0.1 PUBLIC VK_EXT_metal_surface)
endif ()

target_include_directories(incandescent-v0.1 PRIVATE third-party/VulkanMemoryAllocator-master/build/install/include)
target_link_directories(incandescent-v0.1 PRIVATE third-party/VulkanMemoryAllocator-master/build/install/include)
target_include_directories(incandescent-v0.1 PRIVATE third-party/fastgltf-main)
target_link_directories(incandescent-v0.1 PRIVATE third-party/fastgltf-main)
target_include_directories(incandescent-v0.1 PRIVATE third-party/stb-master)
target_link_directories(incandescent-v0.1 PRIVATE third-party/stb-master)
target_include_directories(incandescent-v0.1 PRIVATE third-party/imgui-master)
target_link_directories(incandescent-v0.1 PRIVATE third-party/imgui-master)
target_include_directories(incandescent-v0.1 PRIVATE ./src)
target_link_directories(incandescent-v0.1 PRIVATE ./src)
target_link_libraries(incandescent-v0.1 PRIVATE Vulkan::Vulkan)
target_link_libraries(incandescent-v0.1 PRIVATE Eigen3::Eigen)
target_link_libraries(incandescent-v0.1 PRIVATE SDL2::SDL2)
target_link_libraries(incandescent-v0.1 PRIVATE fmt::fmt-header-only)
