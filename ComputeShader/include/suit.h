#ifndef __SUIT_H__
#define __SUIT_H__

#include <vulkan/vulkan.hpp>

bool isDeviceSuitable(const vk::PhysicalDevice& physical_device);

#endif