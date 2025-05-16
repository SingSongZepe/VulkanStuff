

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <vector>

#include "vk2d.hpp"

typedef unsigned int uint;

// #define SDL_MAIN_HANDLED
#undef main

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(
        "test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN
    );

    if (!window)
    {
        SDL_Log("create window failed");
        exit(2);
    }
    bool shouldClose = false;
    SDL_Event event;

    uint count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());

    // SDL_Vulkan_CreateSurface();

    // result of following code
    // VK_KHR_surface
    // VK_KHR_win32_surface for windows arch
    // for (const char* extension : extensions)
    // {
    //     std::cout << extension << std::endl;
    // }
    
    vk2d::Init(extensions, [&](vk::Instance instance) {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window, instance, &surface))
        {
            throw std::runtime_error("failed to create vulkan surface");
        }
        return surface;
    }, 1024, 720);
    
    auto& renderer = vk2d::getRenderer();

    while (!shouldClose)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                shouldClose = true;
            }
        }
        renderer.Render();
    }

    vk2d::Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

