
#include "tool.hpp"

std::string readWholeFile(const std::string& filename)
{
    // the flag std::ios::ate move the file pointer to the end of the file
    // so that can get the size of file quickly
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    
    if (!file.is_open())
    {
        std::cerr << "read " << filename << " failed." << std::endl;
        return std::string{};
    }

    auto size = file.tellg();
    std::string content;
    content.resize(size);

    file.seekg(0);
    file.read(content.data(), content.size());

    return content;    
}