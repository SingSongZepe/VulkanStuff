#pragma once

#include <vulkan/vulkan.hpp>

namespace vk2d
{
    class Swapchain final
    {
    public:
        vk::SwapchainKHR swapchain;

        Swapchain(int w, int h);
        ~Swapchain();
        
        struct SwapchainInfo 
        {
            vk::Extent2D image_extent;
            uint32_t image_count;
            vk::SurfaceFormatKHR format;
            vk::SurfaceTransformFlagsKHR transform;
            vk::PresentModeKHR present;
        };

        SwapchainInfo info;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> image_views;
        std::vector<vk::Framebuffer> framebuffers;

        void queryInfo(int w, int h);
        void getImage();
        void createImageView();
        void createFramebuffers(int w, int h);
    };

} // namespace vk2d