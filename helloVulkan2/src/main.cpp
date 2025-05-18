
#define GLFW_INCLUDE_VULKAN  
#include <GLFW/glfw3.h>  

#define GLM_FORCE_RADIANS  
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp> // glm::vec4  
#include <glm/mat4x4.hpp> // glm::mat4  
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// define the marco, then include the Defination of functions
// otherwise only prototypes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <vulkan/vulkan.hpp>

#include <chrono>
#include <array>
#include <set>
#include <stdexcept>
#include <iostream>
#include <fstream> 
#include <cstdlib> 
#include <optional>
#include <unordered_map>

#define TEXTURE_FILE_ "./textures/cartethyia.png"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

#define VIKING_ROOM_MODEL_PATH "./models/viking_room.obj"
#define VIKING_ROOM_TEXTURE_PATH "./textures/viking_room.png"

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT_FUNC_NAME_WHEN_CALLED 1
#endif

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

#define DUPLICATED_VERTICES 0

namespace utils
{
static std::vector<char> read_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("failed to load file: " + filename);       
    }

    size_t file_size = file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
}   
} // namespace utils

namespace vk2d
{
struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;

public:      
    Vertex() {
        pos = {0.0f, 0.0f, 0.0f};
        color = {0.0f, 0.0f, 0.0f};
        tex_coord = {0.0f, 0.0f};
    };

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos 
               && color == other.color 
               && tex_coord == other.tex_coord; 
    }

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        vk::VertexInputBindingDescription binding_description;

        binding_description.setBinding(0)
                           .setStride(sizeof(Vertex))
                           .setInputRate(vk::VertexInputRate::eVertex);
        
        return binding_description;
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 3> attribute_descriptions;

#if DEBUG
        std::cout << "the offset of the pos member in Vertex is " << offsetof(Vertex, pos) << std::endl;
#endif

        attribute_descriptions[0].setBinding(0)
                                 .setLocation(0)
                                 .setFormat(vk::Format::eR32G32B32Sfloat)
                                 .setOffset(offsetof(Vertex, pos));

        attribute_descriptions[1].setBinding(0)
                                 .setLocation(1)
                                 .setFormat(vk::Format::eR32G32B32Sfloat)
                                 .setOffset(offsetof(Vertex, color));

        attribute_descriptions[2].setBinding(0)
                                 .setLocation(2)
                                 .setFormat(vk::Format::eR32G32Sfloat)
                                 .setOffset(offsetof(Vertex, tex_coord));

        return attribute_descriptions;
    }
};
}

#if !DUPLICATED_VERTICES
namespace std
{
    template<>
    struct hash<vk2d::Vertex>
    {
        size_t operator()(vk2d::Vertex const& vertex) const 
        {
            // return  ((hash<glm::vec3>()(vertex.pos) ^ 
            //         (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
            //         (hash<glm::vec2>()(vertex.tex_coord) << 1);
            
            return hash<glm::vec3>()(vertex.pos) ^
                   hash<glm::vec3>()(vertex.color) ^
                   hash<glm::vec2>()(vertex.tex_coord);
        }
    };
} // namespace std
#endif

namespace vk2d
{
struct UniformBufferObject
{
    // glm::vec2 foo;
    // alignas(16) glm::mat4 model;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

// const std::vector<Vertex> vertices{
//     {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
//     {{0.5f, -0.5f, 0.0f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//     {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

//     {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
//     {{0.5f, -0.5f, -0.5f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//     {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
// };

// const std::vector<uint16_t> v_indices{
//     0, 1, 2, // triangle 
//     2, 3, 0, 
//     4, 5, 6, 
//     6, 7, 4
// };

bool checkValidationLayerSupported()
{
#if DEBUG
    std::cout << "checkValidationLayerSupported" << " called\n"; 
#endif

    uint32_t layer_count;

    if (vk::enumerateInstanceLayerProperties(&layer_count, nullptr) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to enumerate the instance layer properites");
    };
    std::vector<vk::LayerProperties> available_layers(layer_count);    
    if (vk::enumerateInstanceLayerProperties(&layer_count, available_layers.data()) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to enumerate the instance layer properites");
    };

    for (const auto& layer : validation_layers)
    {
        bool found = false;
        for (const auto& alayer : available_layers)
        {
            if (strcmp(layer, alayer.layerName) == 0)
            {
                found = true;
            }
        } 
        if (!found)
        {
            return false;
        }
    }
    return true;
}

std::vector<const char*> getRequiredExtensions(uint32_t& required_extension_count)
{
#if DEBUG
    std::cout << "getRequiredExtensions" << " called\n";
#endif
    
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    required_extension_count = glfw_extension_count;

    if (enable_validation_layers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        required_extension_count++;
    }

    return extensions;
}

// static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
//     VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
//     VkDebugUtilsMessageTypeFlagsEXT message_type,
//     const VkDebugUtilsMessengerCallbackDataEXT* pcallback_data,
//     void* puser_data)  {
//     std::cerr << "validation layer" << pcallback_data->pMessage << std::endl;

//     return vk::False;
// }

// VkResult createDebugUtilsMessengerEXT(VkInstance instance, 
//                                       const VkDebugUtilsMessengerCreateInfoEXT* pcreate_info,
//                                       const VkAllocationCallbacks* pallocator,
//                                       VkDebugUtilsMessengerEXT* pdebug_messenger) 
// {
//     auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

//     if (func != nullptr)
//     {
//         return func(instance, pcreate_info, pallocator, pdebug_messenger);
//     }
//     else
//     {
//         return VK_ERROR_EXTENSION_NOT_PRESENT;
//     }
// }

// void destroyDebugUtilsMessengerEXT(VkInstance instance,
//                                    VkDebugUtilsMessengerEXT debug_messenger,
//                                    const VkAllocationCallbacks* pallocator)
// {
//     auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    
//     if (func != nullptr)
//     {
//         func(instance, debug_messenger, pallocator);
//     }
// }

class HelloTriangleApplication
{
public:
    void run()
    {
#if DEBUG
        std::cout << "HelloTriangleApplication::run" << " called\n";
#endif
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
        // QueueFamilyIndices()
        // {
        // }

        // QueueFamilyIndices(QueueFamilyIndices&& indices_) 
        // : graphics_family(indices_.graphics_family),
        //   present_family(indices_.present_family)
        // {

        // }

        // QueueFamilyIndices& operator=(QueueFamilyIndices&& indices_) noexcept
        // {
        //     if (this == &indices_)
        //     {
        //         return *this;
        //     }
        //     graphics_family = indices_.graphics_family;
        //     present_family = indices_.present_family;
        //     indices_.graphics_family.reset();
        //     present_family.reset();
        // }

        operator bool() const
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapchainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;
    };

    GLFWwindow* window;
    vk::Instance instance;
    // VkDebugUtilsMessengerEXT debug_messenger;
    vk::SurfaceKHR surface;

    vk::PhysicalDevice physical_device = VK_NULL_HANDLE;

    QueueFamilyIndices indices;
    // logical device
    vk::Device device;

    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchain_images;
    std::vector<vk::ImageView> swapchain_image_views;
    // depth testing
    vk::Image depth_image;
    vk::DeviceMemory depth_image_memory;
    vk::ImageView depth_image_view;

    vk::Format swapchain_image_format;
    vk::Extent2D swapchain_extent;

    vk::Queue graphics_queue;
    vk::Queue present_queue;
    
    // pipeline
    vk::RenderPass render_pass;
    vk::DescriptorSetLayout descriptor_set_layout;
    vk::PipelineLayout pipeline_layout;
    vk::Pipeline graphics_pipeline;

    std::vector<vk::Framebuffer> swapchain_framebuffers;

    // command
    vk::CommandPool command_pool;
    std::vector<vk::CommandBuffer> command_buffers;

    // geometry
    std::vector<Vertex> vertices;
    std::vector<uint32_t> v_indices;
    vk::Buffer vertex_buffer;
    vk::DeviceMemory vertex_buffer_memory;
    vk::Buffer index_buffer;
    vk::DeviceMemory index_buffer_memory;
    
    // descriptor
    vk::DescriptorPool descriptor_pool;
    std::vector<vk::DescriptorSet> descriptor_sets;
    
    std::vector<vk::Buffer> uniform_buffers;
    std::vector<vk::DeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;

    // texture
    uint32_t mip_levels;
    vk::Image texture_image;
    vk::DeviceMemory texture_image_memory;
    vk::ImageView texture_image_view;
    vk::Sampler texture_sampler;
    
    // synchronization
    std::vector<vk::Semaphore> image_available_semaphores;
    std::vector<vk::Semaphore> render_finished_semaphores;
    std::vector<vk::Fence> in_flight_fences;

    bool framebuffer_resized = false;

    // current frame
    // in order to switch between two images, so that GPU will not wait for CPU to render next image
    uint32_t current_frame = 0;

    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // unresizable
        // if the window is resizable, then we should recreate the swapchain
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }


    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebuffer_resized = true;
    }

    void initVulkan()
    {
        createInstance();
        createSurface();
        // setupDebugMessenger();
        pickPhysicalDevice();
        createLogicalDevice();
        getQueue();
        
        createSwapchain();
        createImageViews();
        
        // pipeline
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();

        createCommandPool();
        createCommandBuffers();

        createDepthResoureces();
        createFramebuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();

        // load model
        loadModel();
        // vertex buffer index buffer
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();

        // synchronization
        createSyncObjects();
    }

    void createSurface()
    {
#if DEBUG
        std::cout << "createSurface" << " called" << std::endl;
#endif
        VkSurfaceKHR surface_;
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
        surface = vk::SurfaceKHR(surface_);
    }

    void pickPhysicalDevice()
    {
        uint32_t device_count = 0;
        auto devices = instance.enumeratePhysicalDevices();

        if (devices.size() == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan Support!");
        }

        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                indices = findQueueFamilies(device);
                physical_device = device;
                break;
            }
        }
        
        if (physical_device == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU");
        }        
    }

    void createLogicalDevice()
    {
        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = {
            indices.graphics_family.value(), indices.present_family.value()
        };
        float queue_priority = 1.0f;

        for (uint32_t queue_family : unique_queue_families)
        {
            vk::DeviceQueueCreateInfo queue_create_info;
            queue_create_info.sType = vk::StructureType::eDeviceQueueCreateInfo;
            queue_create_info.setQueueFamilyIndex(indices.graphics_family.value());
            queue_create_info.setQueueCount(1);
        
            queue_create_info.setPQueuePriorities(&queue_priority);
            
            queue_create_infos.push_back(queue_create_info);
        }
        
        vk::PhysicalDeviceFeatures device_features;
        // for anisotropy filtering
        device_features.setSamplerAnisotropy(vk::True);

        vk::DeviceCreateInfo create_info;
        create_info.sType = vk::StructureType::eDeviceCreateInfo;
        create_info.setQueueCreateInfoCount(queue_create_infos.size())
                   .setPQueueCreateInfos(queue_create_infos.data())
                   .setPEnabledFeatures(&device_features)
                   .setEnabledExtensionCount(device_extensions.size())
                   .setPpEnabledExtensionNames(device_extensions.data());

        // validation layer for old implementaion
        // but the new implementation is integrated with validation layer of instance
        if (enable_validation_layers)
        {
            create_info.setEnabledLayerCount(validation_layers.size());
            create_info.setPpEnabledLayerNames(validation_layers.data());
        }
        else
        {
            create_info.setEnabledLayerCount(0);
        }
        
        device = physical_device.createDevice(create_info);
    }

    void getQueue()
    {
        graphics_queue = device.getQueue(indices.graphics_family.value(), 0);
        present_queue = device.getQueue(indices.present_family.value(), 0);
    }

    void createSwapchain()
    {
        auto support_details = querySwapchainSupport(physical_device);
        auto surface_format = chooseSwapchainFormat(support_details.formats);
        auto present_mode = chooseSwapPresentMode(support_details.present_modes);
        auto extent = chooseSwapExtent(support_details.capabilities);
        
        // it's better that request at least one more image than the minimum
        uint32_t image_count = support_details.capabilities.minImageCount + 1;
        // if capabilities.maxImageCount == 0, means there is no maximum, it's a exception
        if (support_details.capabilities.maxImageCount > 0 && image_count > support_details.capabilities.maxImageCount)
        {
            image_count = support_details.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR create_info;
        create_info.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        create_info.setSurface(surface)
                   .setMinImageCount(image_count)
                   .setImageFormat(surface_format.format)
                   .setImageColorSpace(surface_format.colorSpace)
                   .setImageExtent(extent)
                   .setImageArrayLayers(1)
                   .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

        if (indices.graphics_family.value() != indices.present_family.value())
        {
#if DEBUG
            std::cout << "swapchain two queue family\n";
#endif
            std::array<uint32_t, 2> indices_ = {indices.graphics_family.value(), indices.present_family.value()};
            create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
            create_info.setQueueFamilyIndices(indices_);
            create_info.setQueueFamilyIndexCount(indices_.size());
        }
        else
        {
#if DEBUG
            std::cout << "swapchain one queue family\n";
#endif
            create_info.setImageSharingMode(vk::SharingMode::eExclusive);
        }

        create_info.setPreTransform(support_details.capabilities.currentTransform)
                   .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                   .setPresentMode(present_mode)
                   .setClipped(vk::True)
                   // VK_NULL_HANDLE nullptr, ((void*)0), 0ULL, even 0
                   .setOldSwapchain(VK_NULL_HANDLE);

        swapchain = device.createSwapchainKHR(create_info);

        // retrieving images
        swapchain_images = device.getSwapchainImagesKHR(swapchain);

        // store some local variables that will be used in future
        swapchain_image_format = surface_format.format;
        swapchain_extent = extent;
    }

    void createImageViews()
    {
        swapchain_image_views.resize(swapchain_images.size());

        for (int i = 0; i < swapchain_images.size(); i++)
        {
            swapchain_image_views[i] = createImageView(swapchain_images[i], swapchain_image_format, 
                                                       vk::ImageAspectFlagBits::eColor, 1);
        }
    }

    void createDescriptorSetLayout()
    {
        vk::DescriptorSetLayoutBinding ubo_layout_binding;

        ubo_layout_binding.setBinding(0)
                          .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                          .setDescriptorCount(1)
                          .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                          .setPImmutableSamplers(nullptr);

        vk::DescriptorSetLayoutBinding sampler_layout_binding;
        
        sampler_layout_binding.setBinding(1)
                              .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                              .setDescriptorCount(1)
                              .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                              .setPImmutableSamplers(nullptr);

        std::array<vk::DescriptorSetLayoutBinding, 2> bindings{ubo_layout_binding, sampler_layout_binding};

        vk::DescriptorSetLayoutCreateInfo create_info;
        
        create_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
        create_info.setBindingCount(bindings.size())
                   .setPBindings(bindings.data());

        descriptor_set_layout = device.createDescriptorSetLayout(create_info);
    }

    void createGraphicsPipeline()
    {
        // load shader files
        auto vert_shader_bin = utils::read_file("./shader/vert.spv");
        auto frag_shader_bin = utils::read_file("./shader/frag.spv");

        vk::ShaderModule vert_shader_module = createShaderModule(vert_shader_bin);
        vk::ShaderModule frag_shader_module = createShaderModule(frag_shader_bin);

        // vertex shader stage
        vk::PipelineShaderStageCreateInfo vert_shader_stage_info;
        vert_shader_stage_info.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vert_shader_stage_info.setStage(vk::ShaderStageFlagBits::eVertex);
        vert_shader_stage_info.setModule(vert_shader_module);
        // entrypoint main func
        vert_shader_stage_info.setPName("main");

        // fragment shader stage 
        vk::PipelineShaderStageCreateInfo frag_shader_stage_info;
        frag_shader_stage_info.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        frag_shader_stage_info.setStage(vk::ShaderStageFlagBits::eFragment);
        frag_shader_stage_info.setModule(frag_shader_module);
        // entrypoint main func
        frag_shader_stage_info.setPName("main");

        // std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages = {vert_shader_stage_info, frag_shader_stage_info};
        // vk::PipelineShaderStageCreateInfo shader_stages[2] = {vert_shader_stage_info, frag_shader_stage_info};
        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{vert_shader_stage_info, frag_shader_stage_info};
        
        // start to create pipeline
        // 1. vertex input
        auto bind_description = Vertex::getBindingDescription();
        auto attribute_descriptions = Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertex_input_info;

        vertex_input_info.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertex_input_info.setVertexBindingDescriptionCount(1);
        vertex_input_info.setPVertexBindingDescriptions(&bind_description);
        vertex_input_info.setVertexAttributeDescriptionCount(attribute_descriptions.size());
        vertex_input_info.setPVertexAttributeDescriptions(attribute_descriptions.data());

        // 2. input assemblty
        vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
        input_assembly_info.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        input_assembly_info.setTopology(vk::PrimitiveTopology::eTriangleList)
                           .setPrimitiveRestartEnable(vk::False);

        // 3. Viewports and scissors
        vk::Viewport viewport(0, 0, WIDTH, HEIGHT, 0, 1);
        vk::Rect2D scissors({0, 0}, {(uint32_t) WIDTH, (uint32_t) HEIGHT});
        
        vk::PipelineViewportStateCreateInfo viewport_state_info;
        viewport_state_info.setViewports(viewport)
                           .setViewportCount(1)
                           .setScissors(scissors)
                           .setScissorCount(1);

        // 4. rasterization
        vk::PipelineRasterizationStateCreateInfo rasterizer_state_info;
        rasterizer_state_info.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        rasterizer_state_info.setDepthClampEnable(vk::False)
                             .setRasterizerDiscardEnable(vk::False)
                            //  .setPolygonMode(vk::PolygonMode::eLine)
                             .setPolygonMode(vk::PolygonMode::eFill)
                             .setLineWidth(1.0f)
                             .setCullMode(vk::CullModeFlagBits::eBack)
                             .setFrontFace(vk::FrontFace::eCounterClockwise)
                             .setDepthBiasClamp(vk::False);
        
        // 5. multisampling
        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        multisampling.setSampleShadingEnable(vk::False)
                     .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        
        // 6. depth and stencil testing
        vk::PipelineDepthStencilStateCreateInfo depth_stencil_info;
        depth_stencil_info.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
        depth_stencil_info.setDepthTestEnable(vk::True)   // if the depth of new fragment should be compared to the old one
                          .setDepthWriteEnable(vk::True)  // if the new depth value should be written to the depth buffer
                          .setDepthCompareOp(vk::CompareOp::eLess)  // the less, the more closer to camera
                          .setDepthBoundsTestEnable(vk::False)
                          .setMinDepthBounds(0.0f)
                          .setMaxDepthBounds(1.0f)
                          .setStencilTestEnable(vk::False)
                          .setFront({})
                          .setBack({});

        // 7. color blending
        vk::PipelineColorBlendAttachmentState attachment_state;
        attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eR
                                         | vk::ColorComponentFlagBits::eG
                                         | vk::ColorComponentFlagBits::eB
                                         | vk::ColorComponentFlagBits::eA)
                        .setBlendEnable(vk::False);
        
        vk::PipelineColorBlendStateCreateInfo color_blend_state_info;
        color_blend_state_info.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        color_blend_state_info.setLogicOpEnable(vk::False)
                              .setAttachmentCount(1)
                              .setPAttachments(&attachment_state);
        
        // 8. pipeline layout
        vk::PipelineLayoutCreateInfo pipeline_layout_info;    
        pipeline_layout_info.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipeline_layout_info.setSetLayoutCount(1)
                            .setPSetLayouts(&descriptor_set_layout)
                            .setPushConstantRangeCount(0)
                            .setPPushConstantRanges(nullptr);
        
        pipeline_layout = device.createPipelineLayout(pipeline_layout_info);        

        vk::GraphicsPipelineCreateInfo create_info;
        create_info.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        create_info.setStageCount(2)
                   .setPStages(shader_stages.data())
                   .setPVertexInputState(&vertex_input_info)
                   .setPInputAssemblyState(&input_assembly_info)
                   .setPViewportState(&viewport_state_info)
                   .setPRasterizationState(&rasterizer_state_info)
                   .setPMultisampleState(&multisampling)
                   .setPDepthStencilState(&depth_stencil_info)
                   .setPColorBlendState(&color_blend_state_info)
                   .setLayout(pipeline_layout)
                   .setRenderPass(render_pass)
                   .setSubpass(0);

        auto res = device.createGraphicsPipeline(nullptr, create_info);
        if (res.result != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        graphics_pipeline = res.value;

        // after creating the graphic pipeline, the shader modules are allowed to be released
        device.destroyShaderModule(vert_shader_module);
        device.destroyShaderModule(frag_shader_module);
    }

    void createRenderPass()
    {
        vk::AttachmentDescription color_attachment;
        vk::AttachmentDescription depth_attachment;

        color_attachment.setFormat(swapchain_image_format)
                        // MSAA
                        .setSamples(vk::SampleCountFlagBits::e1)
                        .setLoadOp(vk::AttachmentLoadOp::eClear)
                        .setStoreOp(vk::AttachmentStoreOp::eStore)
                        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                        .setInitialLayout(vk::ImageLayout::eUndefined)
                        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
        
        depth_attachment.setFormat(findDepthFormat())
                        .setSamples(vk::SampleCountFlagBits::e1)
                        .setLoadOp(vk::AttachmentLoadOp::eClear)
                        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                        .setInitialLayout(vk::ImageLayout::eUndefined)
                        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentReference color_attachement_ref;
        vk::AttachmentReference depth_attachement_ref;

        color_attachement_ref.setAttachment(0)
                             .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        depth_attachement_ref.setAttachment(1)
                             .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
               .setColorAttachmentCount(1)
               .setPColorAttachments(&color_attachement_ref)
               .setPDepthStencilAttachment(&depth_attachement_ref);

        // packing those two attachments
        std::array<vk::AttachmentDescription, 2> attachments{
            color_attachment, depth_attachment
        };

        // dependency
        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(vk::SubpassExternal)
                  .setDstSubpass(0)
                  .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
                  .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
                  .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        vk::RenderPassCreateInfo create_info;
        create_info.sType = vk::StructureType::eRenderPassCreateInfo;
        create_info.setAttachmentCount(attachments.size())
                   .setPAttachments(attachments.data())
                   .setSubpassCount(1)
                   .setPSubpasses(&subpass)
                   .setDependencyCount(1)
                   .setPDependencies(&dependency);

        render_pass = device.createRenderPass(create_info);
    }

    void createFramebuffers()
    {
        swapchain_framebuffers.resize(swapchain_image_views.size());

        for (int i = 0; i < swapchain_image_views.size(); i++)
        {
            std::vector<vk::ImageView> attachments{
                swapchain_image_views[i],
                depth_image_view,
            };

            vk::FramebufferCreateInfo framebuffer_info;

            framebuffer_info.sType = vk::StructureType::eFramebufferCreateInfo;
            framebuffer_info.setRenderPass(render_pass)
                            .setAttachmentCount(attachments.size())
                            .setPAttachments(attachments.data())
                            .setWidth(swapchain_extent.width)
                            .setHeight(swapchain_extent.height)
                            .setLayers(1);

            swapchain_framebuffers[i] = device.createFramebuffer(framebuffer_info);
        }
    }

    void createCommandPool()
    {
        vk::CommandPoolCreateInfo create_info;
        
        create_info.sType = vk::StructureType::eCommandPoolCreateInfo;
        create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                   .setQueueFamilyIndex(indices.graphics_family.value());

        command_pool = device.createCommandPool(create_info);
    }

    void loadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, VIKING_ROOM_MODEL_PATH))
        {
            throw std::runtime_error(warn + err);
        }

#       if !DUPLICATED_VERTICES
        std::unordered_map<Vertex, uint32_t> unique_vertices;
#       endif

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex;
                
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                    // attrib.texcoords[2 * index.texcoord_index + 1],
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

#               if !DUPLICATED_VERTICES
                if (unique_vertices.count(vertex) == 0)
                {
                    unique_vertices[vertex] = (uint32_t) vertices.size();
                    vertices.push_back(vertex);
                }
                v_indices.push_back(unique_vertices[vertex]); 
#               endif

#               if DUPLICATED_VERTICES
                vertices.push_back(vertex);
                v_indices.push_back(v_indices.size());
#               endif
            }
        }
    }

    void createVertexBuffer()
    {
        size_t size = sizeof(vertices[0]) * vertices.size();

        // create a staging buffer
        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;
        createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     staging_buffer, staging_buffer_memory);

        // map the vertex data to the buffer
        void* data = device.mapMemory(staging_buffer_memory, 0, size);
        memcpy(data, vertices.data(), size);
        device.unmapMemory(staging_buffer_memory);

        createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                     vk::MemoryPropertyFlagBits::eDeviceLocal, vertex_buffer, vertex_buffer_memory);

        copyBuffer(staging_buffer, vertex_buffer, size);

        device.destroyBuffer(staging_buffer);
        device.freeMemory(staging_buffer_memory);        
    }

    void createIndexBuffer()
    {
        size_t size = sizeof(v_indices[0]) * v_indices.size();

        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;

        createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, 
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     staging_buffer, staging_buffer_memory);
        
        void* data = device.mapMemory(staging_buffer_memory, 0, size);
        memcpy(data, v_indices.data(), size);
        device.unmapMemory(staging_buffer_memory);

        createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                     vk::MemoryPropertyFlagBits::eDeviceLocal,
                     index_buffer, index_buffer_memory);
        
        copyBuffer(staging_buffer, index_buffer, size);

        device.destroyBuffer(staging_buffer);
        device.freeMemory(staging_buffer_memory);
    }

    void createUniformBuffers()
    {
        size_t size = sizeof(UniformBufferObject);

        uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniform_buffers_memory.reserve(MAX_FRAMES_IN_FLIGHT);
        uniform_buffers_mapped.reserve(MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            createBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer,
                         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                         uniform_buffers[i], uniform_buffers_memory[i]);
            
            uniform_buffers_mapped[i] = device.mapMemory(uniform_buffers_memory[i], 0, size);
        }
    }

    void createDescriptorPool()
    {
        std::array<vk::DescriptorPoolSize, 2> pool_sizes;

        pool_sizes[0].setType(vk::DescriptorType::eUniformBuffer)
                     .setDescriptorCount((uint32_t) MAX_FRAMES_IN_FLIGHT);

        pool_sizes[1].setType(vk::DescriptorType::eCombinedImageSampler)
                     .setDescriptorCount((uint32_t) MAX_FRAMES_IN_FLIGHT); 

        vk::DescriptorPoolCreateInfo create_info;
        create_info.setPoolSizeCount(pool_sizes.size())
                   .setPPoolSizes(pool_sizes.data())
                   .setMaxSets((uint32_t) MAX_FRAMES_IN_FLIGHT);
        
        descriptor_pool = device.createDescriptorPool(create_info);
    }

    void createDescriptorSets()
    {
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layout);

        vk::DescriptorSetAllocateInfo alloc_info;

        alloc_info.sType = vk::StructureType::eDescriptorSetAllocateInfo;
        alloc_info.setDescriptorPool(descriptor_pool)
                  .setDescriptorSetCount(MAX_FRAMES_IN_FLIGHT)
                  .setPSetLayouts(layouts.data());

        // descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
        descriptor_sets = device.allocateDescriptorSets(alloc_info);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vk::DescriptorBufferInfo buffer_info;

            buffer_info.setBuffer(uniform_buffers[i])
                       .setOffset(0)
                       .setRange(sizeof(UniformBufferObject));
            
            vk::DescriptorImageInfo image_info;
            image_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                      .setImageView(texture_image_view)
                      .setSampler(texture_sampler);

            std::array<vk::WriteDescriptorSet, 2> descriptor_writes;
            descriptor_writes[0].sType = vk::StructureType::eWriteDescriptorSet;
            descriptor_writes[0].setDstSet(descriptor_sets[i])
                                .setDstBinding(0)
                                .setDstArrayElement(0)
                                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                .setDescriptorCount(1)
                                .setPBufferInfo(&buffer_info)
                                .setPImageInfo(nullptr)
                                .setPTexelBufferView(nullptr);
            
            descriptor_writes[1].sType = vk::StructureType::eWriteDescriptorSet;
            descriptor_writes[1].setDstSet(descriptor_sets[i])
                                .setDstBinding(1)
                                .setDstArrayElement(0)
                                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                .setDescriptorCount(1)
                                .setPImageInfo(&image_info);
                                

            device.updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
                            
        }
    }

    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& buffer_memory) 
    {
        // create buffer
        vk::BufferCreateInfo buffer_info;
        buffer_info.sType = vk::StructureType::eBufferCreateInfo;
        buffer_info.setSize(size)
                   .setUsage(usage)
                   .setSharingMode(vk::SharingMode::eExclusive);
        
        buffer = device.createBuffer(buffer_info);

        // create memory
        auto mem_requirements = device.getBufferMemoryRequirements(buffer);
        
        vk::MemoryAllocateInfo alloc_info;

        alloc_info.sType = vk::StructureType::eMemoryAllocateInfo;
        alloc_info.setAllocationSize(mem_requirements.size)
                  .setMemoryTypeIndex(findMemoryType(mem_requirements.memoryTypeBits, properties));

        buffer_memory = device.allocateMemory(alloc_info);        
        device.bindBufferMemory(buffer, buffer_memory, 0);
    }

    void copyBuffer(vk::Buffer& src_buffer, vk::Buffer& dst_buffer, vk::DeviceSize size)
    {
        auto command_buffer = beginSingleTimeCommands(); {
            vk::BufferCopy copy_region;

            copy_region.setSrcOffset(0)
                       .setDstOffset(0)
                       .setSize(size);

            command_buffer.copyBuffer(src_buffer, dst_buffer, copy_region);

        } endSingleTimeCommands(command_buffer);
    }

    void transitionImageLayout(vk::Image image, vk::Format format, 
                               vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t mip_levels)
    {
        auto command_buffer = beginSingleTimeCommands(); {
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;

            range.setBaseMipLevel(0)
                 .setLevelCount(mip_levels)
                 .setBaseArrayLayer(0)
                 .setLayerCount(1);

            if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
            {
                range.setAspectMask(vk::ImageAspectFlagBits::eDepth);

                if (hasStencilComponent(format))
                {
                    range.aspectMask |= vk::ImageAspectFlagBits::eStencil;
                } 
            }
            else
            {
                range.setAspectMask(vk::ImageAspectFlagBits::eColor);
            }

            barrier.sType = vk::StructureType::eImageMemoryBarrier;
            barrier.setOldLayout(old_layout)
                   .setNewLayout(new_layout)
                   .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                   .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                   .setImage(image)
                   .setSubresourceRange(range);

            vk::PipelineStageFlagBits source_stage;
            vk::PipelineStageFlagBits destination_stage;

            if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
            {
                barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                       .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
                
                source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
                destination_stage = vk::PipelineStageFlagBits::eTransfer;
            }
            else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
            {
                barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                       .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                source_stage = vk::PipelineStageFlagBits::eTransfer;
                destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
            }
            else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
            {
                barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
                       .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead 
                                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

                source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
                destination_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
            }
            else 
            {
                throw std::invalid_argument("unsupported layout transition!");
            }

            command_buffer.pipelineBarrier(source_stage, destination_stage, vk::DependencyFlagBits::eByRegion,
                                           0, nullptr, 0, nullptr, 1, &barrier);

        } endSingleTimeCommands(command_buffer);
    }

    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
    {
        auto command_buffer = beginSingleTimeCommands(); {
            vk::BufferImageCopy region;

            vk::ImageSubresourceLayers image_subresource_layers;

            image_subresource_layers.setAspectMask(vk::ImageAspectFlagBits::eColor)
                             .setMipLevel(0)
                             .setBaseArrayLayer(0)
                             .setLayerCount(1);

            region.setBufferOffset(0)
                  .setBufferRowLength(0)
                  .setBufferImageHeight(0)
                  .setImageSubresource(image_subresource_layers);

            region.setImageOffset({0, 0, 0})
                  .setImageExtent({width, height, 1});

            command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        } endSingleTimeCommands(command_buffer);
    }

    uint32_t findMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties mem_properties = physical_device.getMemoryProperties();

        for (int i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if (type_filter & (1 << i) 
            && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void createCommandBuffers()
    {
        command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        vk::CommandBufferAllocateInfo alloc_info;

        alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
        alloc_info.setCommandPool(command_pool)
                  .setLevel(vk::CommandBufferLevel::ePrimary)
                  .setCommandBufferCount((uint32_t)command_buffers.size());
        
        command_buffers = device.allocateCommandBuffers(alloc_info);
    }

    void createDepthResoureces()
    {
        vk::Format depth_format = findDepthFormat();

        createImage(swapchain_extent.width, swapchain_extent.height, 1, depth_format,
                    vk::ImageTiling::eOptimal, 
                    vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
                    depth_image, depth_image_memory);
        
        depth_image_view = createImageView(depth_image, depth_format, vk::ImageAspectFlagBits::eDepth, 1);

        transitionImageLayout(depth_image, depth_format, 
                              vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
    }

    vk::Format findDepthFormat()
    {
        return findSupportedFormat(
            { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
            vk::ImageTiling::eOptimal, 
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
    }

    inline bool hasStencilComponent(const vk::Format& format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }

    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlagBits features)
    {
#       if DEBUG
        std::cout << "findSupportedFormat" << " called" << std::endl;
        std::cout << candidates.size() << std::endl;
#       endif
        for (const auto& format : candidates)
        {
            vk::FormatProperties properties = physical_device.getFormatProperties(format);
#           if DEBUG
            std::cout << 1;
#           endif
            if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
            {
                return format;
            } 
            else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    void createTextureImage()
    {
        // load image from disk
        int tex_width, tex_height, tex_channel;

        stbi_uc* pixels = stbi_load(VIKING_ROOM_TEXTURE_PATH, 
                                    &tex_width, &tex_height, &tex_channel, 
                                    STBI_rgb_alpha);

        mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;
#       if DEBUG
        std::cout << "totally mip levels: " << mip_levels << std::endl;
#       endif

        vk::DeviceSize image_size = tex_width * tex_height * 4;

        if (!pixels)
        {
            throw std::runtime_error("failed to load texture images");
        }

        vk::Buffer staging_buffer;
        vk::DeviceMemory staging_buffer_memory;
        
        createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     staging_buffer, staging_buffer_memory);

        void* data = device.mapMemory(staging_buffer_memory, 0, image_size);
        memcpy(data, pixels, (size_t) image_size);
        device.unmapMemory(staging_buffer_memory);

        stbi_image_free(pixels);

        createImage(tex_width, tex_height, mip_levels, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled,
                    vk::MemoryPropertyFlagBits::eDeviceLocal, texture_image, texture_image_memory);

        transitionImageLayout(texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eTransferDstOptimal, mip_levels);

        copyBufferToImage(staging_buffer, texture_image, (uint32_t) tex_width, (uint32_t) tex_height);

        // transitionImageLayout(texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal,
                            //   vk::ImageLayout::eShaderReadOnlyOptimal, mip_levels);

        generateMipmaps(texture_image, vk::Format::eR8G8B8A8Srgb, tex_width, tex_height, mip_levels);

        device.destroyBuffer(staging_buffer);
        device.freeMemory(staging_buffer_memory);
    }

    void generateMipmaps(vk::Image image, vk::Format image_format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels)
    {
        // check if the image format supports linear blitting
        vk::FormatProperties format_properties = physical_device.getFormatProperties(image_format);

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }


        vk::CommandBuffer command_buffer = beginSingleTimeCommands(); {
            vk::ImageMemoryBarrier barrier;
            
            vk::ImageSubresourceRange range;
            range.setAspectMask(vk::ImageAspectFlagBits::eColor)
                 .setBaseArrayLayer(0)
                 .setLayerCount(1)
                 .setLevelCount(1);

            barrier.sType = vk::StructureType::eImageMemoryBarrier;
            barrier.setImage(image)
                   .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                   .setDstQueueFamilyIndex(vk::QueueFamilyIgnored);

            int32_t mip_width = tex_width, mip_height = tex_height;
            
            for (int i = 1; i < mip_levels; i++)
            {
                range.setBaseMipLevel(i - 1);
                barrier.setSubresourceRange(range)
                       .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                       .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                       .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                       .setDstAccessMask(vk::AccessFlagBits::eTransferRead);

                command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                               vk::DependencyFlagBits::eByRegion,
                                               0, nullptr,
                                               0, nullptr,
                                               1, &barrier);

                vk::ImageBlit blit;

                vk::ImageSubresourceLayers src_subresource;
                src_subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setMipLevel(i - 1)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1);

                vk::ImageSubresourceLayers dst_subresource;
                dst_subresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setMipLevel(i)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1);

                blit.setSrcOffsets(std::array<vk::Offset3D, 2>{{{0, 0, 0}, {mip_width, mip_height, 1}}})
                    .setSrcSubresource(src_subresource)
                    .setDstOffsets(std::array<vk::Offset3D, 2>{{{0, 0, 0}, {mip_width > 1 ? mip_width / 2 : 1, 
                                                                            mip_height > 1 ? mip_height / 2 : 1, 1}}})
                    .setDstSubresource(dst_subresource);
                
                command_buffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, 
                                         image, vk::ImageLayout::eTransferDstOptimal,
                                         1, &blit, vk::Filter::eLinear);

                barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                       .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                       .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                       .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                               vk::DependencyFlagBits::eByRegion,
                                               0, nullptr,
                                               0, nullptr,
                                               1, &barrier);
                
                if (mip_width > 1) mip_width /= 2;
                if (mip_height > 1) mip_height /= 2;
            }

            barrier.subresourceRange.setBaseMipLevel(mip_levels - 1);
            barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                   .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                   .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                   .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                           vk::DependencyFlagBits::eByRegion,
                                           0, nullptr,
                                           0, nullptr,
                                           1, &barrier);

        } endSingleTimeCommands(command_buffer);
    }

    void createTextureImageView()
    {
        texture_image_view = createImageView(texture_image, vk::Format::eR8G8B8A8Srgb, 
                                             vk::ImageAspectFlagBits::eColor, mip_levels);
    }

    void createTextureSampler()
    {
        // retrieve the max anisotropy
        auto properties = physical_device.getProperties();

        vk::SamplerCreateInfo create_info;

        create_info.sType = vk::StructureType::eSamplerCreateInfo;
        create_info.setMagFilter(vk::Filter::eLinear)
                   .setMinFilter(vk::Filter::eLinear)
                   .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                   .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                   .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                   .setAnisotropyEnable(vk::True)
                   .setMaxAnisotropy(properties.limits.maxSamplerAnisotropy)
                   .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                   .setUnnormalizedCoordinates(vk::False)
                   .setCompareEnable(vk::False)
                   .setCompareOp(vk::CompareOp::eAlways)
                   .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                   .setMipLodBias(0.0f)
                   .setMinLod(0.0f)
                   .setMaxLod((float) mip_levels);
        
        texture_sampler = device.createSampler(create_info);
    }

    vk::ImageView createImageView(vk::Image image, vk::Format format, 
                                  vk::ImageAspectFlagBits aspect_flags, uint32_t mip_levels)
    {
        vk::ImageViewCreateInfo create_info;

        vk::ImageSubresourceRange range;

        range.setAspectMask(aspect_flags)
             .setBaseMipLevel(0)
             .setLevelCount(mip_levels)
             .setBaseArrayLayer(0)
             .setLayerCount(1);

        create_info.sType = vk::StructureType::eImageViewCreateInfo;
        create_info.setImage(image)
                   .setViewType(vk::ImageViewType::e2D)
                   .setFormat(format)
                   .setSubresourceRange(range);
        
        return device.createImageView(create_info);
    }

    vk::CommandBuffer beginSingleTimeCommands()
    {
        vk::CommandBufferAllocateInfo alloc_info;
        
        alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
        alloc_info.setLevel(vk::CommandBufferLevel::ePrimary)
                  .setCommandPool(command_pool)
                  .setCommandBufferCount(1);
                
        vk::CommandBuffer command_buffer = device.allocateCommandBuffers(alloc_info)[0];

        vk::CommandBufferBeginInfo begin_info;

        begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;
        begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        command_buffer.begin(begin_info);

        return command_buffer;
    }

    void endSingleTimeCommands(vk::CommandBuffer& command_buffer)
    {
        command_buffer.end();

        vk::SubmitInfo submit_info;
        submit_info.sType = vk::StructureType::eSubmitInfo;
        submit_info.setCommandBufferCount(1)
                   .setPCommandBuffers(&command_buffer);

        graphics_queue.submit(submit_info);
        graphics_queue.waitIdle();

        device.freeCommandBuffers(command_pool, command_buffer);
    }

    void createImage(uint32_t width, uint32_t height, uint32_t mip_levels, 
                     vk::Format format, vk::ImageTiling tiling,
                     vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, 
                     vk::Image& image, vk::DeviceMemory image_memory)
    {
        vk::ImageCreateInfo image_info;

        image_info.sType = vk::StructureType::eImageCreateInfo;
        image_info.setImageType(vk::ImageType::e2D)
                  .setExtent(vk::Extent3D((uint32_t)width, (uint32_t)height, 1))
                  .setMipLevels(mip_levels)
                  .setArrayLayers(1)
                  .setFormat(format)
                  // for efficient accessment for shader
                  .setTiling(tiling)
                  .setInitialLayout(vk::ImageLayout::eUndefined)
                  .setUsage(usage)
                  .setSharingMode(vk::SharingMode::eExclusive)
                  .setSamples(vk::SampleCountFlagBits::e1);

        image = device.createImage(image_info);

        auto mem_requirements = device.getImageMemoryRequirements(image);
        
        vk::MemoryAllocateInfo alloc_info;
        
        alloc_info.sType = vk::StructureType::eMemoryAllocateInfo;

        alloc_info.setAllocationSize(mem_requirements.size);
        alloc_info.setMemoryTypeIndex(findMemoryType(mem_requirements.memoryTypeBits, properties));

        image_memory = device.allocateMemory(alloc_info);

        device.bindImageMemory(image, image_memory, 0);
    }

    void recordCommandBuffer(vk::CommandBuffer& command_buffer, uint32_t image_idx)
    {
        vk::CommandBufferBeginInfo begin_info;
        
        begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;
        begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        
        command_buffer.begin(begin_info); {
            vk::RenderPassBeginInfo render_pass_begin_info;

            // clearvalue: area out of render area
            std::array<vk::ClearValue, 2> clear_colors;
            clear_colors[0].setColor({0.1f, 0.1f, 0.1f, 1.0f});
            clear_colors[1].setDepthStencil({1.0f, 0});

            render_pass_begin_info.setRenderPass(render_pass)
                                  .setFramebuffer(swapchain_framebuffers[image_idx])
                                  .setRenderArea(vk::Rect2D({0, 0}, swapchain_extent))
                                  .setClearValueCount(clear_colors.size())
                                  .setPClearValues(clear_colors.data());
            
            command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline); {
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
                
                vk::Buffer vertex_buffers[] = {vertex_buffer};
                vk::DeviceSize offsets[] = {0};
                command_buffer.bindVertexBuffers(0, vertex_buffer, offsets);
                command_buffer.bindIndexBuffer(index_buffer, 0, vk::IndexType::eUint32);
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, 1, &descriptor_sets[current_frame], 0, nullptr);

                // command_buffer.draw((uint32_t) vertices.size(), 1, 0, 0);
                command_buffer.drawIndexed((uint32_t) v_indices.size(), 1, 0, 0, 0);

            } command_buffer.endRenderPass();
        } command_buffer.end();


    }

    void createSyncObjects()
    {
        image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
        
        vk::SemaphoreCreateInfo semaphore_info;
        semaphore_info.sType = vk::StructureType::eSemaphoreCreateInfo;

        vk::FenceCreateInfo fence_info;
        fence_info.sType = vk::StructureType::eFenceCreateInfo;
        // fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            image_available_semaphores[i] = device.createSemaphore(semaphore_info);
            render_finished_semaphores[i] = device.createSemaphore(semaphore_info);
            in_flight_fences[i] = device.createFence(fence_info);
        }
    }

    void recreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        device.waitIdle();

        cleanupSwapchain();
        createSwapchain();
        createDepthResoureces();
        createImageViews();
        createFramebuffers();
    }

    void cleanupSwapchain()
    {
        device.destroyImageView(depth_image_view);
        device.destroyImage(depth_image);
        device.freeMemory(depth_image_memory);

        for (auto framebuffer : swapchain_framebuffers)
        {
            device.destroyFramebuffer(framebuffer);
        }

        for (auto image_view : swapchain_image_views)
        {
            device.destroyImageView(image_view);
        }

        device.destroySwapchainKHR(swapchain);
    }

    vk::ShaderModule createShaderModule(const std::vector<char>& code)
    {
        vk::ShaderModuleCreateInfo create_info;

        create_info.sType = vk::StructureType::eShaderModuleCreateInfo;
        create_info.setCodeSize(code.size())
                   .setPCode(reinterpret_cast<const uint32_t*>(code.data()));
        
        return device.createShaderModule(create_info);
    }

    bool isDeviceSuitable(const vk::PhysicalDevice& device)
    {
#if DEBUG
        std::cout << "HelloTriangleApplication::isDeviceSuitable" << " called\n";
#endif
        QueueFamilyIndices indices =  findQueueFamilies(device);

        // should support swapchain
        bool extensions_supported = checkDeviceExtensionSupport(device);
        bool swapchain_adequate = false;
        if (extensions_supported)
        {
            auto support_details = querySwapchainSupport(device);
            swapchain_adequate = !support_details.formats.empty() && !support_details.present_modes.empty();
        }

        // should support geometry shader
        auto properties = device.getProperties();
        auto features = device.getFeatures();

        // Intergrated GPU
        // or 
        // Discrete GPu
        return indices 
        && swapchain_adequate
        && properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu 
        && features.geometryShader
        && features.samplerAnisotropy;
    }

    bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device)
    {
        auto available_extensions = device.enumerateDeviceExtensionProperties();
        std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
        
        for (const auto& extension : available_extensions)
        {
            required_extensions.erase(extension.extensionName);
        }

        return required_extensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device)
    {
        QueueFamilyIndices indices;

        VkBool32 present_support = false;

        auto properties = device.getQueueFamilyProperties();
        for (int i = 0; i < properties.size(); i++)
        {
            const auto& property = properties[i];
            if (property.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                indices.graphics_family = i;
            }
            if (device.getSurfaceSupportKHR(i, surface))
            {
                indices.present_family = i;           
            }
            if (indices)
            {
                break;
            }
        }

        return indices;
    }

    SwapchainSupportDetails querySwapchainSupport(const vk::PhysicalDevice& device)
    {
#if DEBUG
        std::cout << "querySwapchainSupport" << " called" << std::endl;
#endif
        SwapchainSupportDetails details;
        
        details.formats = device.getSurfaceFormatsKHR(surface);
        details.present_modes = device.getSurfacePresentModesKHR(surface);
        details.capabilities = device.getSurfaceCapabilitiesKHR(surface);

        return details;
    }

    vk::SurfaceFormatKHR chooseSwapchainFormat(const std::vector<vk::SurfaceFormatKHR>& available_formats)
    {
        for (const auto& format : available_formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Srgb 
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return format;
            }
        }

        return available_formats[0];
    }

    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& available_present_modes)
    {
        for (const auto& present_mode : available_present_modes)
        {
            if (present_mode == vk::PresentModeKHR::eMailbox)
            {
                return present_mode;
            }
        }
        // when the application runs on a mobile device
        // it's important to save energy, FIFO is a good choice.
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;   
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            vk::Extent2D actual_extent = {
                (uint32_t) width,
                (uint32_t) height
            };

            actual_extent.width = std::clamp<uint32_t>(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp<uint32_t>(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actual_extent;
        }
    }

    // Debug messenger
    /*
    // void setupDebugMessenger()
    // {
    //     if (!enable_validation_layers) return;

    //     VkDebugUtilsMessengerCreateInfoEXT create_info{};

    //     create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    //     create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    //                                   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    //                                   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        
    //     create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    //                               | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    //                               | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    //     create_info.pfnUserCallback = debugCallback;
    //     create_info.pUserData = nullptr;

    //     if (createDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to set up debug messenger!");
    //     }
    // }
    */

    void createInstance()
    {
#if DEBUG
        std::cout << "createInstance" << " called" << std::endl;
#endif
        vk::ApplicationInfo app_info;
        app_info.sType = vk::StructureType::eApplicationInfo;
        app_info.pApplicationName = "Hello Triangle";
        // set or directly assign
        // app_info.setPApplicationName("Hello Triangle");

        app_info.pEngineName = "No Engine";
        
        app_info.engineVersion = vk::makeVersion(1, 0, 0);
        // app_info.setEngineVersion(vk::makeVersion(1, 0, 0));
        
        app_info.apiVersion = vk::ApiVersion10;
        // app_info.setApiVersion(VK_VERSION_1_3)

        vk::InstanceCreateInfo create_info;
        create_info.sType = vk::StructureType::eInstanceCreateInfo;
        create_info.setPApplicationInfo(&app_info);

        // extension
        uint32_t required_extension_count = 0;
        auto required_extensions = getRequiredExtensions(required_extension_count);
        
        // VK_KHR_surface VK_KHR_win32_surface
        // for (int i = 0; i < glfw_extension_count; i++)
        // {
        //     std::cout << glfw_extensions[i];
        // }

        uint32_t extension_count = 0;
        vk::enumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<vk::ExtensionProperties> extensions(extension_count);
        vk::enumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        // check whether glfw_extensions are supported?
        for (int i = 0; i < required_extension_count; i++)
        {
            bool found = false;
            for (const auto& extension : extensions)
            {
                if (strcmp(required_extensions[i], extension.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }            
            if (!found)
            {
                throw std::runtime_error("glfw needs extension " + std::string(required_extensions[i]) + " but not supported!");
            }
        }

        create_info.setEnabledExtensionCount(required_extension_count);
        create_info.setPpEnabledExtensionNames(required_extensions.data());

        // layer
        create_info.setEnabledLayerCount(0);
        if (enable_validation_layers && !checkValidationLayerSupported())
        {
            throw std::runtime_error("Enabled validation layer but not supported!");
        }   

        if (enable_validation_layers)
        {
            create_info.setEnabledLayerCount((uint32_t)validation_layers.size());
            create_info.setPpEnabledLayerNames(validation_layers.data());
        }
        else
        {
            create_info.setEnabledLayerCount(0);
        }

        instance = vk::createInstance(create_info);
    }

    void mainLoop()
    {
        while(!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            drawFrame();
        }

        device.waitIdle();
    }

    void drawFrame()
    {  
#if DEBUG
        // std::cout << "current frame index: " << current_frame << std::endl;
        // auto result_ = device.getFenceStatus(in_flight_fences[current_frame]);  

        // if (result_ == vk::Result::eSuccess) {  
            // std::cout << "Fence is signaled." << std::endl;  
        // } else if (result_ == vk::Result::eNotReady) {  
            // std::cout << "Fence is not signaled." << std::endl;  
        // } else {  
            // std::cerr << "Failed to check fence status: " << result_ << std::endl;  
        // }
#endif

        // update uniform buffer
        updateUniformBuffer(current_frame);

        uint32_t image_idx;
        
        auto result = device.acquireNextImageKHR(swapchain, 
                                       std::numeric_limits<uint64_t>::max(), 
                                       image_available_semaphores[current_frame], 
                                       VK_NULL_HANDLE, 
                                       &image_idx);
        
        if (result == vk::Result::eErrorOutOfDateKHR 
        || result == vk::Result::eSuboptimalKHR 
        || framebuffer_resized)
        {
            recreateSwapchain();
            framebuffer_resized = false;
            return;
        }
        else if (result != vk::Result::eSuccess) 
        {
            throw std::runtime_error("failed to acquire the next image");
        }

        command_buffers[current_frame].reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        recordCommandBuffer(command_buffers[current_frame], image_idx);
        
        vk::SubmitInfo submit_info;
        submit_info.sType = vk::StructureType::eSubmitInfo;

        std::vector<vk::Semaphore> wait_semaphores{image_available_semaphores[current_frame]};
        std::vector<vk::PipelineStageFlags> wait_stages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
        
        submit_info.setWaitSemaphoreCount(wait_semaphores.size())
                   .setPWaitSemaphores(wait_semaphores.data())
                   .setPWaitDstStageMask(wait_stages.data())
                   .setCommandBufferCount(1)
                   .setCommandBuffers(command_buffers[current_frame]);

        std::vector<vk::Semaphore> signal_semaphores{render_finished_semaphores[current_frame]};

        submit_info.setSignalSemaphoreCount(signal_semaphores.size())
                   .setPSignalSemaphores(signal_semaphores.data());

        graphics_queue.submit(submit_info, in_flight_fences[current_frame]);

        vk::PresentInfoKHR present_info;
        present_info.setWaitSemaphoreCount(1)
                    .setPWaitSemaphores(signal_semaphores.data())
                    .setPImageIndices(&image_idx)
                    .setSwapchainCount(1)
                    .setSwapchains(swapchain);
        
        result = present_queue.presentKHR(present_info);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            recreateSwapchain();
        }
        else if (result != vk::Result::eSuccess)
        {
            std::cerr << "image present failed" << std::endl;
        }

        if (device.waitForFences(in_flight_fences[current_frame], vk::True, std::numeric_limits<uint32_t>::max()) != vk::Result::eSuccess)
        {
            std::cerr << "wait for fence failed" << std::endl;
        }

        device.resetFences(in_flight_fences[current_frame]);

        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void updateUniformBuffer(uint32_t current_frame)
    {
        static auto start_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();

        // float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
        float time = 0.0f;

        UniformBufferObject ubo;
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_extent.width / (float) swapchain_extent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        // 
        memcpy(uniform_buffers_mapped[current_frame], &ubo, sizeof(ubo));
    }
    
    void cleanup()
    {
        // if (enable_validation_layers) 
        // {
        //     destroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
        // }
        
        cleanupSwapchain();

        device.destroyImageView(texture_image_view);
        device.destroyImage(texture_image);
        device.freeMemory(texture_image_memory);

        device.destroySampler(texture_sampler);

        device.destroyBuffer(vertex_buffer);
        device.freeMemory(vertex_buffer_memory);

        device.destroyBuffer(index_buffer);
        device.freeMemory(index_buffer_memory);

        for (int i = 0; i< MAX_FRAMES_IN_FLIGHT; i++)
        {
            device.destroyBuffer(uniform_buffers[i]);
            device.freeMemory(uniform_buffers_memory[i]);
        }
        device.destroyDescriptorPool(descriptor_pool);
        device.destroyDescriptorSetLayout(descriptor_set_layout);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            device.destroySemaphore(image_available_semaphores[i]);
            device.destroySemaphore(render_finished_semaphores[i]);
            device.destroyFence(in_flight_fences[i]);
        }

        device.destroyCommandPool(command_pool);

        device.destroyPipeline(graphics_pipeline);

        device.destroyRenderPass(render_pass);

        device.destroyPipelineLayout(pipeline_layout);

        device.destroy();

        instance.destroySurfaceKHR(surface);

        instance.destroy();  
        
        glfwDestroyWindow(window);
    
        glfwTerminate();
    }
};
} // namespace vk2d

int main(int argc, char** argv)  
{  
    vk2d::HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;  
}