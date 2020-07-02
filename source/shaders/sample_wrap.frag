#version 430

layout(binding = 0) uniform sampler2D tex;
layout(location = 0) out vec4 color;

void main()
{
	vec2 texcoord = gl_FragCoord.xy / vec2(64.0);
	texcoord = texcoord * vec2(2.0) - vec2(1.0);
	texcoord *= vec2(1.5);
	color = texture(tex, texcoord);
}
