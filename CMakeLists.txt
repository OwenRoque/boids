cmake_minimum_required(VERSION 3.16)
project(BoidsLab LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

set(GLAD_DIR "${CMAKE_SOURCE_DIR}/third_party/glad_gen")

add_executable(boids_lab
    src/main.cpp
    src/Boid.cpp
    src/Simulation.cpp
    src/Renderer.cpp
    ${GLAD_DIR}/src/glad.c
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)

target_include_directories(boids_lab PRIVATE
    src
    ${GLAD_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends)

target_link_libraries(boids_lab PRIVATE
    OpenGL::GL
    glfw
    glm::glm)

target_compile_definitions(boids_lab PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

if(UNIX AND NOT APPLE)
    target_link_libraries(boids_lab PRIVATE dl pthread)
endif()

add_custom_command(TARGET boids_lab POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:boids_lab>/shaders
    COMMENT "Copiando shaders al directorio de salida")
