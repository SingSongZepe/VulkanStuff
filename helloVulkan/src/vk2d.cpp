
#include "vk2d.hpp"

namespace vk2d
{
    void Init(const std::vector<const char*>& extensions,
         CreateSurfaceFunc create_surface_func, int w, int h)
    {
        Context::Init(extensions, create_surface_func);
        Context::GetInstance().initSwapchain(w, h);
        Shader::Init(readWholeFile(VERTEX_SHADER_SPV_PATH), readWholeFile(FRAG_SHADER_SPV_PATH));
        
        Context::GetInstance().render_process->initRenderPass();
        Context::GetInstance().render_process->initLayout();
        Context::GetInstance().swapchain->createFramebuffers(w, h);
        Context::GetInstance().render_process->InitPipeline(w, h);

        Context::GetInstance().initRenderer();
    }

    void Quit()
    {
        Context::GetInstance().device.waitIdle();

        Context::GetInstance().destoryRenderer();

        Context::GetInstance().render_process.reset();
        Shader::Quit();
        Context::GetInstance().destorySwapchain();

        Context::Quit();
    }
} // namespace vk2d