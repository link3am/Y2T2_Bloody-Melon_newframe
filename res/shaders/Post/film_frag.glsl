#version 420

layout(location = 0) in vec2 inUV;

out vec4 frag_color;

layout (binding = 0) uniform sampler2D s_screenTex;

uniform float u_Intensity = 0.6;
uniform float u_Time = 0.0;
void main() 
{
	

	float x = (inUV.x + 4) * (inUV.y + 4) * (sin(u_Time) * 10);
	vec4 grain = u_Intensity * vec4(mod((mod(x, 13) + 1) * (mod(x, 123) + 1), 0.01) - 0.005);

	frag_color = texture(s_screenTex, inUV) + grain;

}

