cmake_minimum_required(VERSION 3.8)
project(Cresellia)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL -DIMGUI_DEFINE_MATH_OPERATORS)

include_directories(lib/imgui lib/bstream lib/Palkia/include include)
add_subdirectory(lib)

file(GLOB_RECURSE CRESSELIA_SRC
    "src/*.cpp"
    "src/*.c"
    "include/*.hpp"
    "include/*.h"
    
    "lib/ImGuiFileDialog/ImGuiFileDialog/*.h"
    "lib/imgui/misc/cpp/*.cpp"
    "lib/bStream/bstream.cpp"
)

add_executable(cresselia ${CRESSELIA_SRC})
target_include_directories(cresselia PUBLIC include "${PROJECT_SOURCE_DIR}/lib" lib/ImGuiFileDialog/ImGuiFileDialog lib/glfw/include lib/Palkia/include)

target_link_libraries(cresselia PUBLIC palkia imgui ImGuiFileDialog glfw)
