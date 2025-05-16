#version 450

// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5,  0.5),
//     vec2(-0.5, 0.5)
// );

// vec3 colors[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_tex_coord;

layout(binding = 0) uniform UniformBufferObject {
    // vec2 foo;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 fragment_color;
layout(location = 1) out vec2 fragment_tex_coord;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 0.0, 1.0);
    fragment_color = in_color;
    fragment_tex_coord = in_tex_coord;
}   