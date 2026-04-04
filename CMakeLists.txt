cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME "WishEngine")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

project(${PROJECT_NAME})

file(GLOB_RECURSE SRC_C CONFIGURE_DEPENDS ./src/*.c)
file(GLOB_RECURSE SRC_CPP CONFIGURE_DEPENDS ./src/*.cpp)
file(GLOB_RECURSE SRC_HEADERS CONFIGURE_DEPENDS ./src/*.hpp)

add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_C} ${SRC_HEADERS})

set(THIRD_PARTY_DIR ./submodules/)

# GLFW #
set(GLFW_DIR ${THIRD_PARTY_DIR}/glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_DIR}/include)
add_subdirectory(${GLFW_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glfw)
# -- #

# GLM #
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY ON CACHE BOOL "" FORCE)
set(DGLM_BUILD_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/glm ${CMAKE_CURRENT_BINARY_DIR}/glm)
# -- #

# VMA #
# marked as SYSTEM to suppress warnings
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${THIRD_PARTY_DIR}/VMA/include)
add_subdirectory(${THIRD_PARTY_DIR}/VMA ${CMAKE_CURRENT_BINARY_DIR}/VMA)
# -- #

# KTX
# add_subdirectory(${THIRD_PARTY_DIR}/KTX/lib ${CMAKE_CURRENT_BINARY_DIR}/KTX)
# -- #

# Slang #
find_library(Slang_LIBRARY NAMES slang slang-compiler HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
# -- #

# IMGUI #
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
add_library(imgui STATIC)
target_sources(imgui
    PRIVATE
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
# -- #


# include_directories(SYSTEM $ENV{VULKAN_SDK}/Include)
find_package(Vulkan REQUIRED)


target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rendering
        ${CMAKE_CURRENT_SOURCE_DIR}/src/camera
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm
    imgui
    # ktx
    GPUOpen::VulkanMemoryAllocator
    # $ENV{VULKAN_SDK}/Lib/vulkan-1.lib
    Vulkan::Vulkan
    ${Slang_LIBRARY}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLFW_INCLUDE_NONE
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./"
    # GLM_FORCE_DEPTH_ZERO_TO_ONE
)
