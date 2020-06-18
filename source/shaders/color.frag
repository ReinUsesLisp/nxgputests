#version 420

layout(location = 1) in vec4 color;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = color;
}
