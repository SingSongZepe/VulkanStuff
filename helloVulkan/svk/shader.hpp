#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>

namespace vk2d
{
    class Shader final
    {
    public:
        static void Init(const std::string& vertex_source, const std::string& fragment_source);
        static void Quit();
        
        static Shader& GetInstance();

        vk::ShaderModule vertex_module;
        vk::ShaderModule fragment_module;

        std::vector<vk::PipelineShaderStageCreateInfo> getStage();

        ~Shader();
    private:
        static std::unique_ptr<Shader> ist;

        Shader(const std::string& vertex_source, const std::string& fragment_source);

        std::vector<vk::PipelineShaderStageCreateInfo> stage_;
        void initStage();
    };
}
