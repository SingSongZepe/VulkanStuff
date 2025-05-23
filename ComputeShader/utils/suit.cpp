
#include "suit.h"

bool isDeviceSuitable(const vk::PhysicalDevice& physical_device)
{
    return physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
}