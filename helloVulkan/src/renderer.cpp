
#include "renderer.hpp"
#include "context.hpp"

namespace vk2d
{
    Renderer::Renderer()
    {
        initCmdPool();
        allocCmdBuffer();
        createSems();
        createFence();
    }

    Renderer::~Renderer()
    {
        auto& device = Context::GetInstance().device;
        device.destroyFence(cmd_avaliable_fence);
        device.freeCommandBuffers(cmd_pool, cmd_buffer);
        device.destroyCommandPool(cmd_pool);
        device.destroySemaphore(image_avaliable);
        device.destroySemaphore(image_draw_finish);
    }

    void Renderer::initCmdPool()
    {
        vk::CommandPoolCreateInfo create_info;

        create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        cmd_pool = Context::GetInstance().device.createCommandPool(create_info);
    }
    
    void Renderer::allocCmdBuffer()
    {
        vk::CommandBufferAllocateInfo alloc_info;

        alloc_info.setCommandPool(cmd_pool)
                  .setCommandBufferCount(1)
                  .setLevel(vk::CommandBufferLevel::ePrimary);
        
        cmd_buffer = Context::GetInstance().device.allocateCommandBuffers(alloc_info)[0];
    }

    void Renderer::Render()
    {
        auto& device = Context::GetInstance().device;
        auto& swapchain = Context::GetInstance().swapchain;
        auto& render_process = Context::GetInstance().render_process;

        auto result = device.acquireNextImageKHR(swapchain->swapchain,
                                   std::numeric_limits<uint64_t>::max());

        if (result.result != vk::Result::eSuccess)
        {
            std::cerr << "acquire next image failed!" << std::endl;
        }

        auto image_index = result.value;

        cmd_buffer.reset();

        vk::CommandBufferBeginInfo begin;
        begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        cmd_buffer.begin(begin); {
            cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, render_process->pipeline);
            vk::RenderPassBeginInfo render_pass_begin;
            vk::Rect2D area;
            area.setOffset({0, 0})
                .setExtent(swapchain->info.image_extent);

            vk::ClearValue clear_value;
            clear_value.color = vk::ClearColorValue(std::array<float, 4>({0.1f, 0.1f, 0.1f, 1.0f}));

            render_pass_begin.setRenderPass(render_process->render_pass)
                             .setRenderArea(area)
                             .setFramebuffer(swapchain->framebuffers[image_index])
                             .setClearValues(clear_value);

            cmd_buffer.beginRenderPass(render_pass_begin, {}); {
                cmd_buffer.draw(3, 1, 0, 0);
            } cmd_buffer.endRenderPass();
        } cmd_buffer.end();

        // send the command to the gpu
        vk::SubmitInfo submit;
        submit.setCommandBuffers(cmd_buffer);

        auto& graphics_queue = Context::GetInstance().graphics_queue;
        auto& present_queue = Context::GetInstance().present_queue;

        graphics_queue.submit(submit, cmd_avaliable_fence);

        vk::PresentInfoKHR present;
        present.setImageIndices(image_index)
               .setSwapchains(swapchain->swapchain);
        
        if (present_queue.presentKHR(present) != vk::Result::eSuccess)
        {
            std::cerr << "image present failed" << std::endl;
        }

        if (Context::GetInstance().device.waitForFences(cmd_avaliable_fence, true, std::numeric_limits<uint64_t>::max())
            != vk::Result::eSuccess)
        {
            std::cerr << "wait for fence failed" << std::endl;
        }

        Context::GetInstance().device.resetFences(cmd_avaliable_fence);
    }

    void Renderer::createSems()
    {
        vk::SemaphoreCreateInfo create_info;

        image_avaliable = Context::GetInstance().device.createSemaphore(create_info);
        image_draw_finish = Context::GetInstance().device.createSemaphore(create_info);
    }

    void Renderer::createFence()
    {
        vk::FenceCreateInfo create_info;
        
        cmd_avaliable_fence = Context::GetInstance().device.createFence(create_info);
    }
}
