#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragment_color;

void main()
{
    outColor = vec4(fragment_color, 1.0);
}


