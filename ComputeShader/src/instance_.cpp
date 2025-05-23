#include "instance_.h"

#include <iostream>
#include "global.h"

namespace cs
{
std::unique_ptr<Instance> Instance::inst_ = nullptr;

Instance::Instance()
{
    // init vulkan
    createInstance();    
}

Instance::~Instance()
{

}

void Instance::createInstance()
{
#   if DEBUG
    std::cout << "createInstance called\n";
#   endif
    // extensions
    auto extensions = vk::enumerateInstanceExtensionProperties();
    for (const auto& ext : extensions)
    {
        std::cout << ext.extensionName << std::endl;
    }
    
    vk::ApplicationInfo app_info;
    app_info.sType = vk::StructureType::eApplicationInfo;
    app_info.setApiVersion(vk::ApiVersion10)
            .setPApplicationName("application cs")
            .setPEngineName("cs")
            .setEngineVersion(vk::makeVersion(1, 0, 0));



    vk::InstanceCreateInfo create_info;
    create_info.sType = vk::StructureType::eInstanceCreateInfo;
    create_info.setPpEnabledExtensionNames();

}

void Instance::init()
{
    inst_.reset(new Instance());
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


