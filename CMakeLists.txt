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
include_directories(${GLFW_DIR}/include)
add_subdirectory(${GLFW_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glfw)
# -- #

# GLM #
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY ON CACHE BOOL "" FORCE)
set(DGLM_BUILD_INSTALL ON CACHE BOOL "" FORCE)
# add_definitions(-D GLM_ENABLE_EXPERIMENTAL) ## for GLM
add_subdirectory(${THIRD_PARTY_DIR}/glm ${CMAKE_CURRENT_BINARY_DIR}/glm)
# include_directories("submodules/glm/glm")
# -- #

# VMA #
add_subdirectory(${THIRD_PARTY_DIR}/VMA ${CMAKE_CURRENT_BINARY_DIR}/VMA)
include_directories(${THIRD_PARTY_DIR}/include)

# -- #

# KTX
add_subdirectory(${THIRD_PARTY_DIR}/KTX/lib ${CMAKE_CURRENT_BINARY_DIR}/KTX)
# -- #

# Slang #
find_library(Slang_LIBRARY NAMES slang slang-compiler HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
# -- #


include_directories(SYSTEM $ENV{VULKAN_SDK}/Include)


target_include_directories(${PROJECT_NAME}
    PRIVATE
        C:/VulkanSDK/1.4.341.1/Include
        ./include
        # ./src/game/
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm
    ktx
    GPUOpen::VulkanMemoryAllocator
    $ENV{VULKAN_SDK}/Lib/vulkan-1.lib
    ${Slang_LIBRARY}
)

# target_link_libraries(${PROJECT_NAME} PUBLIC $ENV{VULKAN_SDK}/Lib)

# target_compile_definitions(myEngine PUBLIC
#     RESSOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
# )

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./"
)
