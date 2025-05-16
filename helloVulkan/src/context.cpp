
#include "context.hpp"

namespace vk2d
{
    std::unique_ptr<Context> Context::ist = nullptr; // Define the static member  

    void Context::Init(const std::vector<const char*>& extensions, 
        CreateSurfaceFunc create_surface_func)
    {   
        ist.reset(new Context(extensions, create_surface_func));

    }
    
    void Context::Quit()
    {
        ist.reset();
    }

    Context& Context::GetInstance()
    {
        return *ist;
    }

    Context::Context(const std::vector<const char*>& extensions,
         CreateSurfaceFunc create_surface_func)
    {
        createInstance(extensions);
        pickupPhysicalDevice();
        surface = create_surface_func(instance);
        if (!surface)
        {
            throw std::runtime_error("surface is null");
        }
        queryQueueFamilyIndices();
        createDevice();
        getQueue();
        render_process.reset(new RenderProcess);
    }

    void Context::initSwapchain(int w, int h)
    {
        swapchain.reset(new Swapchain(w, h));        
    }

    void Context::destorySwapchain()
    {
        swapchain.reset();
    }

    void Context::initRenderer()
    {
        renderer.reset(new Renderer);
    }

    void Context::destoryRenderer()
    {
        renderer.reset();
    }

    Context::~Context()
    {
        instance.destroySurfaceKHR(surface);
        device.destroy();
        instance.destroy();
    };

    void Context::createInstance(const std::vector<const char*>& extensions)
    {
        vk::InstanceCreateInfo create_info;

        // to package all registeration info up and
        // pass it into createInstance function 
        vk::ApplicationInfo app_info;
        // optional, version of vulkan API
        app_info.setApiVersion(VK_API_VERSION_1_3);

        // two things: layer and extension
        // validation layers
        // vk::enumerateInstanceLayerProperties()
        std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
            
        create_info.setPApplicationInfo(&app_info)
                   .setPEnabledLayerNames(layers)
                   .setPEnabledExtensionNames(extensions);

        instance = vk::createInstance(create_info);
    }

    void Context::pickupPhysicalDevice()
    {
        auto devices = instance.enumeratePhysicalDevices();
        // for (auto& device: devices)
        // {
        //     auto feature = device.getFeatures();
        //     if (feature.geometryShader)
        //     {
        //         // some 
        //     }
        // }

        physical_device = devices[0];

        std::cout << physical_device.getProperties().deviceName << std::endl;
    }
    
    void Context::createDevice() 
    {
        std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        
        vk::DeviceCreateInfo create_info;        
        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        
        // 0.0 - 1.0f;
        float priorities = 1.0f;
        
        if (queue_family_indices.graphics_queue.value() == queue_family_indices.present_queue.value())
        {
            vk::DeviceQueueCreateInfo queue_create_info;
            queue_create_info.setPQueuePriorities(&priorities)
                             .setQueueCount(1)
                             .setQueueFamilyIndex(queue_family_indices.graphics_queue.value());
            queue_create_infos.push_back(std::move(queue_create_info));
        } 
        else 
        {
            vk::DeviceQueueCreateInfo queue_create_info;
            queue_create_info.setPQueuePriorities(&priorities)
                             .setQueueCount(1)
                             .setQueueFamilyIndex(queue_family_indices.graphics_queue.value());
            queue_create_infos.push_back(queue_create_info);
            queue_create_info.setPQueuePriorities(&priorities)
                             .setQueueCount(1)
                             .setQueueFamilyIndex(queue_family_indices.present_queue.value());
            queue_create_infos.push_back(std::move(queue_create_info));
        }
        
        // std::vector<T> or T or other linear structure
        create_info.setQueueCreateInfos(queue_create_infos)
                   .setPEnabledExtensionNames(extensions);

        device = physical_device.createDevice(create_info);
    }

    void Context::queryQueueFamilyIndices()
    {
        auto properties = physical_device.getQueueFamilyProperties();
        for (int i = 0; i < properties.size(); i++)
        {
            const auto& property = properties[i];
            // find the first queue family that supports graphics   
            if (property.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                queue_family_indices.graphics_queue = i;
                
            }
            if (physical_device.getSurfaceSupportKHR(i, surface))
            {
                queue_family_indices.present_queue = i;
            }
            // call the bool() of the object to assess its value
            if (queue_family_indices)
            {
                break;
            }
        }   
    }

    void Context::getQueue()
    {
        graphics_queue = device.getQueue(queue_family_indices.graphics_queue.value(), 0);
        present_queue = device.getQueue(queue_family_indices.present_queue.value(), 0);        
    }
}