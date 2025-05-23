#ifndef __INSTANCE_H___
#define __INSTANCE_H___

#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

#include "global.h"

namespace cs
{
class Instance
{
private:
    static std::unique_ptr<Instance> inst_;
    vk::Instance inst;

    vk::PhysicalDevice physical_device;
    vk::Device device;
    vk::Queue compute_queue;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> compute_queue;

        operator bool() const
        {
            return compute_queue.has_value();
        }
    };
    
    QueueFamilyIndices qf_indices;

    QueueFamilyIndices findQueueFamily(const vk::PhysicalDevice& physical_device);

    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void getComputeQueue();

    Instance();
public:

    ~Instance();
    static void init();
    static void quit();
    static Instance& getInstance();
};
}

#endif // __INSTANCE_H___