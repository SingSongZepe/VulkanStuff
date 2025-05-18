#version 450

layout(location = 0) in vec3 fragment_color;
layout(location = 1) in vec2 fragment_tex_coord;

layout(location = 0) out vec4 out_color;
layout(binding = 1) uniform sampler2D tex_sampler;

void main()
{
    out_color = vec4(texture(tex_sampler, fragment_tex_coord).rgb, 1.0);
}
