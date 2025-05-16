
#include "shader.hpp"
#include "context.hpp"

namespace vk2d
{
    std::unique_ptr<Shader> Shader::ist = nullptr;
    
    void Shader::Init(const std::string& vertex_source, const std::string& fragment_source)
    {
        ist.reset(new Shader(vertex_source, fragment_source));
    }

    void Shader::Quit()
    {
        ist.reset();
    }

    Shader::Shader(const std::string& vertex_source, const std::string& fragment_source)
    {
        vk::ShaderModuleCreateInfo create_info;
        create_info.codeSize = vertex_source.size();
        create_info.pCode = (uint32_t*) vertex_source.data(); 
        
        vertex_module = Context::GetInstance().device.createShaderModule(create_info);

        create_info.codeSize = fragment_source.size();
        create_info.pCode = (uint32_t*) fragment_source.data(); 
        
        fragment_module = Context::GetInstance().device.createShaderModule(create_info);

        initStage();
    }

    Shader::~Shader()
    {
        auto& device = Context::GetInstance().device;
        device.destroyShaderModule(vertex_module);
        device.destroyShaderModule(fragment_module);
    }

    Shader& Shader::GetInstance()
    {
        return *ist;
    }

    std::vector<vk::PipelineShaderStageCreateInfo> Shader::getStage()
    {
        return stage_;
    }

    void Shader::initStage()
    {
        stage_.resize(2);
        stage_[0].setStage(vk::ShaderStageFlagBits::eVertex)
                 .setModule(vertex_module)
                 .setPName("main");
        stage_[1].setStage(vk::ShaderStageFlagBits::eFragment)
                 .setModule(fragment_module)
                 .setPName("main");
    }
}