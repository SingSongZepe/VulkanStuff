#pragma once

#include <vulkan/vulkan.hpp>


namespace vk2d
{
    class Renderer final
    {
    public:
        Renderer();
        ~Renderer();

        void Render();

    private:
        vk::CommandPool cmd_pool;
        vk::CommandBuffer cmd_buffer;

        vk::Semaphore image_avaliable;
        vk::Semaphore image_draw_finish;
        vk::Fence cmd_avaliable_fence;

        void initCmdPool();
        void allocCmdBuffer();
        void createSems();
        void createFence();
    };
}
