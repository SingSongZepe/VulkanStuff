
cmake_minimum_required(VERSION 3.10)

project(computshader
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

option(MODE_TEST "Build the test version" OFF) 

if (MODE_TEST)
    file(GLOB SOURCE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
    )

    find_package(Vulkan REQUIRED)

    add_executable(cs ${SOURCE_FILES})

    target_link_libraries(cs PRIVATE Vulkan::Vulkan)

else()
    file(GLOB SOURCE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    find_package(Vulkan REQUIRED)

    # find_program(GLSLC_COMPLIER glslc REQUIRED)
    # execute_process(COMMAND ${GLSLC_COMPLIER} ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/vert.spv)
    # execute_process(COMMAND ${GLSLC_COMPLIER} ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_CURRENT_SOURCE_DIR}/shader/frag.spv)

    add_executable(cs ${SOURCE_FILES})

    target_link_libraries(cs PRIVATE Vulkan::Vulkan)

endif()