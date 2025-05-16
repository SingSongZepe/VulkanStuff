
#include "swapchain.hpp"
#include "context.hpp"

namespace vk2d
{
    Swapchain::Swapchain(int w, int h)
    {
        queryInfo(w, h);

        vk::SwapchainCreateInfoKHR create_info;
        create_info.setClipped(true)
                   .setImageArrayLayers(1)
                   .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                   .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                   .setSurface(Context::GetInstance().surface)
                   .setImageColorSpace(info.format.colorSpace)
                   .setImageFormat(info.format.format)
                   .setImageExtent(info.image_extent)
                   .setMinImageCount(info.image_count)
                   .setPresentMode(info.present);

        auto& queue_indices = Context::GetInstance().queue_family_indices;
        if (queue_indices.graphics_queue.value() == queue_indices.present_queue.value())
        {
            create_info.setQueueFamilyIndices(queue_indices.graphics_queue.value())
                       .setImageSharingMode(vk::SharingMode::eExclusive);
        }
        else
        {
            std::array indices = {queue_indices.graphics_queue.value(), queue_indices.present_queue.value()};
            create_info.setQueueFamilyIndices(indices)
                       .setImageSharingMode(vk::SharingMode::eConcurrent);
        }

        swapchain = Context::GetInstance().device.createSwapchainKHR(create_info);


        getImage();
        createImageView();
    }
    
    Swapchain::~Swapchain()
    {
        auto& device = Context::GetInstance().device;
        for (auto& framebuffer : framebuffers)
        {
            device.destroyFramebuffer(framebuffer);
        }
        for (auto& view : image_views)
        {
            device.destroyImageView(view);
        }
        device.destroySwapchainKHR(swapchain);
    }

    void Swapchain::queryInfo(int w, int h)
    {
        auto& physical_device = Context::GetInstance().physical_device;
        auto& surface = Context::GetInstance().surface;
        auto formats = physical_device.getSurfaceFormatsKHR(surface);
        info.format = formats[0];

        for (const auto& format : formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                info.format = format;
                break;            
            }
        }

        auto capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
        info.image_count = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);

        info.image_extent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        info.image_extent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        info.transform = capabilities.currentTransform;

        auto presents = physical_device.getSurfacePresentModesKHR(surface);
        
        // default
        info.present = vk::PresentModeKHR::eFifo;
        for (const auto& present : presents)
        {
            if (present == vk::PresentModeKHR::eMailbox)
            {
                info.present = present;
                break;
            }
        }        
    }

    void Swapchain::getImage()
    {
        images = Context::GetInstance().device.getSwapchainImagesKHR(swapchain);
    }

    void Swapchain::createImageView()
    {
        image_views.resize(images.size());

        for (int i = 0; i < images.size(); i++)
        {
            vk::ImageViewCreateInfo create_info;
            vk::ComponentMapping mapping;
            vk::ImageSubresourceRange range;
            
            range.setBaseMipLevel(0)
                 .setLevelCount(1)
                 .setBaseArrayLayer(0)
                 .setLayerCount(1)
                 .setAspectMask(vk::ImageAspectFlagBits::eColor);

            create_info.setImage(images[i])
                       .setViewType(vk::ImageViewType::e2D)
                       .setComponents(mapping)
                       .setFormat(info.format.format)
                       .setSubresourceRange(range);

            image_views[i] = Context::GetInstance().device.createImageView(create_info);
        }
    }

    void Swapchain::createFramebuffers(int w, int h)
    {
        framebuffers.resize(images.size());
        for (int i = 0; i < framebuffers.size(); i++)
        {
            vk::FramebufferCreateInfo create_info;
            create_info.setAttachments(image_views[i])
                       .setWidth(w)
                       .setHeight(h)
                       .setRenderPass(Context::GetInstance().render_process->render_pass)
                       .setLayers(1);
            
            framebuffers[i] = Context::GetInstance().device.createFramebuffer(create_info);
        }
    }
}

