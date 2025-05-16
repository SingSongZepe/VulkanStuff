
#include "render_process.hpp"
#include "shader.hpp"
#include "context.hpp"
#include "swapchain.hpp"

namespace vk2d
{
    void RenderProcess::InitPipeline(int width, int height)
    {
        vk::GraphicsPipelineCreateInfo create_info;

        // 1. Vertex Input
        vk::PipelineVertexInputStateCreateInfo input_state;
        create_info.setPVertexInputState(&input_state);

        // 2. Vertex Assembly
        vk::PipelineInputAssemblyStateCreateInfo input_asm;
        input_asm.setPrimitiveRestartEnable(false)
                 .setTopology(vk::PrimitiveTopology::eTriangleList);
        create_info.setPInputAssemblyState(&input_asm);

        // 3. Shader
        auto stages = Shader::GetInstance().getStage();
        create_info.setStages(stages);

        // 4 viewport
        vk::PipelineViewportStateCreateInfo viewport_state;
        vk::Viewport viewport(0, 0, width, height, 0.1, 1);
        vk::Rect2D rect({0, 0}, 
                        {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
        
        viewport_state.setViewports(viewport)
                      .setScissors(rect);

        create_info.setPViewportState(&viewport_state);

        // 5. rasterization
        vk::PipelineRasterizationStateCreateInfo rast_info;
        rast_info.setRasterizerDiscardEnable(false)
                 .setCullMode(vk::CullModeFlagBits::eBack)
                 .setFrontFace(vk::FrontFace::eCounterClockwise)
                 // if eFill, then a filled triangle
                 // if eLine, then a line frame triangle
                //  .setPolygonMode(vk::PolygonMode::eFill)
                 .setPolygonMode(vk::PolygonMode::eFill)
                 .setLineWidth(1)
                 .setDepthBiasEnable(vk::False);

        create_info.setPRasterizationState(&rast_info);

        // 6. multisample
        vk::PipelineMultisampleStateCreateInfo multisample;
        multisample.setSampleShadingEnable(false)
                   .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        create_info.setPMultisampleState(&multisample);

        // 7. test depth test, stencil test ...

        // 8. color blending
        vk::PipelineColorBlendStateCreateInfo blend;
        vk::PipelineColorBlendAttachmentState attachs;

        attachs.setBlendEnable(false)
               .setColorWriteMask(vk::ColorComponentFlagBits::eA |
                                  vk::ColorComponentFlagBits::eB |
                                  vk::ColorComponentFlagBits::eG |
                                  vk::ColorComponentFlagBits::eR);

        blend.setLogicOpEnable(false)
             .setAttachments(attachs);
        create_info.setPColorBlendState(&blend);
        
        // 9. render pass and layout
        create_info.setRenderPass(render_pass)
                   .setLayout(layout);

        auto res = Context::GetInstance().device.createGraphicsPipeline(nullptr, create_info);
        if (res.result != vk::Result::eSuccess)
        {
            throw std::runtime_error("create graphics pipeline failed");
        }
        pipeline = res.value;
    }
    
    void RenderProcess::initLayout()
    {
        vk::PipelineLayoutCreateInfo create_info;
        layout = Context::GetInstance().device.createPipelineLayout(create_info);
    }

    void RenderProcess::initRenderPass()
    {
        vk::RenderPassCreateInfo create_info;
        vk::AttachmentDescription attach_desc;
        
        attach_desc.setFormat(Context::GetInstance().swapchain->info.format.format)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eNone)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setSamples(vk::SampleCountFlagBits::e1);

        create_info.setAttachments(attach_desc);
        // [0, 1, 2]

        vk::AttachmentReference reference;
        reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
                 // use the attach_desc count, there is only one attach_desc
                 .setAttachment(0);

        vk::SubpassDescription subpass_desc;
        subpass_desc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                    .setColorAttachments(reference);
        create_info.setSubpasses(subpass_desc);

        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                  .setDstSubpass(0)
                  .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                  .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                  .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

        create_info.setDependencies(dependency);
        render_pass = Context::GetInstance().device.createRenderPass(create_info);
    }

    RenderProcess::~RenderProcess()
    {
        auto& device = Context::GetInstance().device;
        device.destroyRenderPass(render_pass);
        device.destroyPipelineLayout(layout);
        device.destroyPipeline(pipeline);
    }
}   