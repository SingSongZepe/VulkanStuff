
cmake_minimum_required(VERSION 3.10)

project(helloVulkan2
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

option(MODE_TEST "Build the test version" OFF) 

if (MODE_TEST)
file(GLOB SOURCE_FILES 
 ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stb-master/stb_image.h
 )

find_package(Vulkan REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/stb-master)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)

add_executable(vk2d ${SOURCE_FILES})

target_link_libraries(vk2d PRIVATE Vulkan::Vulkan)

else()
file(GLOB SOURCE_FILES 
 ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stb-master/stb_image.h
 ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader/tiny_obj_loader.h
 )

find_package(Vulkan REQUIRED)

find_program(GLSLC_COMPLIER glslc REQUIRED)
execute_process(COMMAND ${GLSLC_COMPLIER} ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/vert.spv)
execute_process(COMMAND ${GLSLC_COMPLIER} ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/frag.spv)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4.bin.WIN64/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/stb-master)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4.bin.WIN64/lib-mingw-w64)

add_executable(vk2d ${SOURCE_FILES})

target_link_libraries(vk2d PRIVATE Vulkan::Vulkan)
target_link_libraries(vk2d PRIVATE glfw3)

endif()