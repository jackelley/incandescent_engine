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

add_executable(incandescent-v0.1 src/main.cpp
        src/incandescent_engine.cpp
        src/incandescent_engine.h
        src/incandescent_types.h
)

# Set where the ImGui files are stored
set(IMGUI_PATH "/Users/jackkelley/code_packages/imgui-master")

# Compile as static library
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
add_library(ImGui STATIC ${IMGUI_SOURCES}
        src/incandescent_types.h)
target_include_directories(ImGui PUBLIC ${IMGUI_PATH})

target_include_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/code_packages/VulkanMemoryAllocator-master/build/install/include)
target_link_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/code_packages/VulkanMemoryAllocator-master/build/install/include)
target_include_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/code_packages/fastgltf-main)
target_link_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/code_packages/fastgltf-main)
target_include_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/code_packages/stb-master)
target_link_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/code_packages/stb-master)
target_include_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/CLionProjects/incandescent-v0.1/src)
target_link_directories(incandescent-v0.1 PRIVATE /Users/jackkelley/CLionProjects/incandescent-v0.1/src)
target_link_libraries(incandescent-v0.1 PRIVATE Vulkan::Vulkan)
target_link_libraries(incandescent-v0.1 PRIVATE Eigen3::Eigen)
target_link_libraries(incandescent-v0.1 PRIVATE SDL2::SDL2)
target_link_libraries(incandescent-v0.1 PUBLIC ImGui)
target_link_libraries(incandescent-v0.1 PRIVATE fmt::fmt-header-only)
