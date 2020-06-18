#version 420

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 texcoord;

layout(location = 1) out vec4 out_color;
layout(location = 2) out vec4 out_texcoord;

void main()
{
    gl_Position = position;
    out_color = color;
    out_texcoord = texcoord;
}
