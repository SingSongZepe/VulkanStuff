#pragma once

#include <vulkan/vulkan.hpp>
#include "context.hpp"
#include "shader.hpp"
#include "global.hpp"

namespace vk2d
{   
    void Init(const std::vector<const char*>& extensions,
         CreateSurfaceFunc createSurfaceFunc, int w, int h);
    void Quit();

    inline Renderer& getRenderer()
    {
        return *Context::GetInstance().renderer;
    }
} // namespace vk2d
