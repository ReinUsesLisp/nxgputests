#version 420

layout(location = 0) in vec4 color;

layout(location = 1) out vec4 out_color;

void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 0.0, 1.0);
    out_color = color;
}
