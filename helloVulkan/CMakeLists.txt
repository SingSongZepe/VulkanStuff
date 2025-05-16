
cmake_minimum_required(VERSION 3.10)

project(helloVulkan 
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/svk)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB SOURCE 
 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
 )

# vulkan 
find_package(Vulkan REQUIRED)
    
find_program(GLSLC_COMPLIER glslc REQUIRED)
execute_process(COMMAND ${GLSLC_COMPLIER} ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/vert.spv)
execute_process(COMMAND ${GLSLC_COMPLIER} ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/frag.spv)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/SDL/include/SDL2)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/SDL2/bin)

add_executable(vk2d ${SOURCE})

target_link_libraries(vk2d PRIVATE Vulkan::Vulkan)
target_link_libraries(vk2d PRIVATE SDL2)


