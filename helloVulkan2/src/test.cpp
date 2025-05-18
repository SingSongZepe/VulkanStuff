
#include <glm/glm.hpp>
#include <glm/vec4.hpp> // glm::vec4  
#include <glm/mat4x4.hpp> // glm::mat4 

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <stdio.h>
#include <iostream>
#include <unordered_map>

// struct Vertex
// {
//     glm::vec3 pos;
//     glm::vec3 color;
//     glm::vec2 tex_coord;

// public:      
//     Vertex() {
//         pos = glm::vec3(0.0);
//         color = glm::vec3(0.0);
//         tex_coord = glm::vec2(0.0);
//     };

//     bool operator==(const Vertex& other)
//     {
//         return pos == other.pos && color == other.color && tex_coord == other.tex_coord; 
//     }
// };

struct Student
{
    int age;
    std::string name;
    // float height;
    // float weight;
    glm::vec3 color;

    bool operator==(const Student& stu) const
    {
        return age == stu.age 
               && name == stu.name 
            //    && height == stu.height 
            //    && weight == stu.height;
               && color == stu.color;
    }
};

namespace std
{
    // template<>
    // struct hash<Vertex>
    // {
    //     size_t operator()(Vertex const& vertex) const 
    //     {
    //         return  ((hash<glm::vec3>()(vertex.pos) ^ 
    //                 (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
    //                 (hash<glm::vec2>()(vertex.tex_coord) << 1);
    //     }
    // };

    template<>
    struct hash<Student>
    {
        size_t operator()(Student const& stu) const
        {
            return hash<int>()(stu.age) ^
                   hash<std::string>()(stu.name) ^
                //    hash<float>()(stu.height) ^
                //    hash<float>()(stu.weight);
                   hash<glm::vec3>()(stu.color);
        }
    };
    
} // namespace std

void test1()
{
    // std::unordered_map<Vertex, uint32_t> unique_vertices;
    
    // Vertex v;
    // v.pos = {0.0f, 0.0f, 1.0f};
    // v.color = {0.0f, 0.0f, 1.0f};
    // v.tex_coord = {0.0f, 1.0f};

    // if (unique_vertices.count(v) == 0)
    // {
    //     std::cout << "0 v" << std::endl;
    // }

    std::unordered_map<Student, uint32_t> students;

    Student stu;
    stu.age = 18;
    stu.name = "Ginko";
    // stu.height = 1.7f;
    // stu.weight = 121.3f;
    stu.color = glm::vec3(1.0f);

    if (students.count(stu) == 0)
    {
        std::cout << "0 stu" << std::endl; 
    }

    students[stu] = static_cast<uint32_t>(1);

    std::cout << students[stu];
}


int main()
{
    printf("Test File\n");

    test1();

    return 0;
}