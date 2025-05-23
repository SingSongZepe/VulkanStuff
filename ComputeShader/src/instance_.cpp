#include "instance_.h"

#include <iostream>
#include "global.h"
#include "register.h"
#include "suit.h"

namespace cs
{
std::unique_ptr<Instance> Instance::inst_ = nullptr;

Instance::Instance()
{
    // init vulkan
    createInstance();
    pickPhysicalDevice();
    createLogicalDevice();
    getComputeQueue();
}

Instance::~Instance()
{
    device.destroy();

    inst.destroy();
}

Instance::QueueFamilyIndices Instance::findQueueFamily(const vk::PhysicalDevice& physical_device)
{
    DEBUG_MESSAGE_NORMAL("findQueueFamily")
    
    Instance::QueueFamilyIndices indices;
    auto queue_properties = physical_device.getQueueFamilyProperties();

    for (int i = 0; i < queue_properties.size(); i++)
    {
        const auto& property = queue_properties[i];
        if (property.queueFlags & vk::QueueFlagBits::eCompute)
        {
            indices.compute_queue = i;
        }
        if (indices)
        {
            break;
        }
    }

    return indices;
}

void Instance::createLogicalDevice()
{
    DEBUG_MESSAGE_NORMAL("createLogicalDevice")

    const float priorities = 1.0f;
    vk::DeviceQueueCreateInfo queue_create_info;
    queue_create_info.setQueueCount(1)
                     .setQueueFamilyIndex(qf_indices.compute_queue.value())
                     .setPQueuePriorities(&priorities);

    vk::DeviceCreateInfo create_info;
    create_info.sType = vk::StructureType::eDeviceCreateInfo;
    create_info.setQueueCreateInfoCount(1)
               .setPQueueCreateInfos(&queue_create_info)
               .setEnabledExtensionCount(0)
               .setPEnabledExtensionNames(nullptr);

    device = physical_device.createDevice(create_info);
}

void Instance::getComputeQueue()
{
    DEBUG_MESSAGE_NORMAL("getComputeQueue")

    compute_queue = device.getQueue(qf_indices.compute_queue.value(), 0);
}

void Instance::pickPhysicalDevice()
{
    DEBUG_MESSAGE_NORMAL("pickPhysicalDevice")
    
    auto candidate_devices = inst.enumeratePhysicalDevices();
    
    for (const auto& candidate : candidate_devices)
    {        
        if (isDeviceSuitable(candidate))
        {
            qf_indices = findQueueFamily(candidate);
            physical_device = candidate;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("no device matches the requirements!");
    }

    DEBUG_MESSAGE_NORMAL(physical_device.getProperties().deviceName);
}

void Instance::createInstance()
{
    DEBUG_MESSAGE_NORMAL("createInstance called")
    // extensions
    // auto extensions = vk::enumerateInstanceExtensionProperties();
    // for (const auto& ext : extensions)
    // {
        // std::cout << ext.extensionName << std::endl;
    // }
    
    // required extensions
    auto extensions = getRequiredExtensions();

    // required layers
    auto layers = getRequiredLayers();

    if (ENABLE_validation_layers)
    {
        const size_t layers_count = layers.size();
        layers.resize(layers.size() + validation_layers.size());

        for (size_t i = 0; i < validation_layers.size(); i++)
        {
            layers[i + layers_count] = validation_layers[i];
        }
    }

    vk::ApplicationInfo app_info;
    app_info.sType = vk::StructureType::eApplicationInfo;
    app_info.setApiVersion(vk::ApiVersion10)
            .setPApplicationName("application cs")
            .setPEngineName("cs")
            .setEngineVersion(vk::makeVersion(1, 0, 0));

    vk::InstanceCreateInfo create_info;
    create_info.sType = vk::StructureType::eInstanceCreateInfo;
    create_info.setEnabledExtensionCount(extensions.size())
               .setPpEnabledExtensionNames(extensions.data())
               .setEnabledLayerCount(layers.size())
               .setPpEnabledLayerNames(layers.data())
               .setPApplicationInfo(&app_info);

    inst = vk::createInstance(create_info);
}

void Instance::init()
{
    inst_.reset(new Instance);
}

void Instance::quit()
{
    inst_.reset();
}

Instance& Instance::getInstance()
{
    return *inst_;
}
} // namespace cs


