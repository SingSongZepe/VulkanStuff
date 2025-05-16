#pragma once

#include <vulkan/vulkan.hpp>

namespace vk2d
{
    class RenderProcess final
    {
    public:
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::RenderPass render_pass;

        ~RenderProcess();

        void initLayout();
        void initRenderPass();
        void InitPipeline(int width, int height);
    };
}