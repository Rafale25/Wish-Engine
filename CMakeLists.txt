cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME "WishEngine")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
project(${PROJECT_NAME})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) // slower compilation, more optimization

file(GLOB_RECURSE SRC_C CONFIGURE_DEPENDS ./src/*.c)
file(GLOB_RECURSE SRC_CPP CONFIGURE_DEPENDS ./src/*.cpp)
file(GLOB_RECURSE SRC_HEADERS CONFIGURE_DEPENDS ./src/*.hpp)

add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_C} ${SRC_HEADERS})

# target_include_directories(${PROJECT_NAME}
#     PRIVATE
#         ./src/game/
# )

# target_compile_definitions(myEngine PUBLIC
#     RESSOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
# )

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./"
)
