#ifndef __REGISRER_H__
#define __REGISRER_H__

#include <vulkan/vulkan.hpp>
#include <vector>
#include <iostream>

#include "global.h"

std::vector<const char*> getRequiredExtensions();

std::vector<const char*> getRequiredLayers();


#endif // __REGISRER_H__