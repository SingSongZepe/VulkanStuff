#pragma once

#include <vulkan/vulkan.hpp>
#include <functional>
#include <fstream>
#include <iostream>

using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

std::string readWholeFile(const std::string& filename);

