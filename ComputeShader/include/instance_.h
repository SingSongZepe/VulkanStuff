
#include <memory>

#include <vulkan/vulkan.hpp>

namespace cs
{
class Instance
{
private:
    static std::unique_ptr<Instance> inst_;
    vk::Instance inst;
    void createInstance();

    Instance();
public:

    ~Instance();
    static void init();
    static void quit();
    static Instance& getInstance();
};
}