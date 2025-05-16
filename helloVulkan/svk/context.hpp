#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <iostream>
#include <optional>
#include "tool.hpp"
#include "swapchain.hpp"
#include "render_process.hpp"
#include "renderer.hpp"

namespace vk2d
{
    class Context final
    {
    public:
        static void Init(const std::vector<const char*>& extensions, 
            CreateSurfaceFunc create_surface_func);
        static void Quit();
        static Context& GetInstance();

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphics_queue;
            std::optional<uint32_t> present_queue;

            operator bool() const {
                return graphics_queue.has_value() && present_queue.has_value();
            }
        };

        vk::Instance instance;
        vk::PhysicalDevice physical_device;
        vk::Device device;
        vk::Queue graphics_queue;
        vk::Queue present_queue;
        vk::SurfaceKHR surface;
        std::unique_ptr<Swapchain> swapchain;
        std::unique_ptr<RenderProcess> render_process;
        std::unique_ptr<Renderer> renderer;
        QueueFamilyIndices queue_family_indices;

        void initSwapchain(int, int);
        void destorySwapchain();
        void initRenderer();
        void destoryRenderer();

        ~Context();
    private:
        static std::unique_ptr<Context> ist;
        
        Context(const std::vector<const char*>& extensions, 
            CreateSurfaceFunc create_surface_func);

        void createInstance(const std::vector<const char*>& extensions);
        void pickupPhysicalDevice();
        void createDevice();
        void queryQueueFamilyIndices();
        void getQueue();

    };
} // vk2d
