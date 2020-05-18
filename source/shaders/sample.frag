#version 430

layout(binding = 0) uniform sampler2D tex;
layout(location = 0) out vec4 color;

const vec2 CANVAS_SIZE = vec2(64.0, 64.0);

void main()
{
	vec2 texcoord = gl_FragCoord.xy / CANVAS_SIZE;
	color = texture(tex, texcoord);
}
