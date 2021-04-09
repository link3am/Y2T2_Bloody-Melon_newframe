#version 420

uniform sampler2D diffuseTexture;

in vec2 fUV;

out vec4 frag_color;

uniform float u_Intensity = 0.005;

void main()
{
	vec2 uv = floor((fUV - 0.5) / u_Intensity) * u_Intensity;

	frag_color = texture(diffuseTexture, uv + 0.5);
}