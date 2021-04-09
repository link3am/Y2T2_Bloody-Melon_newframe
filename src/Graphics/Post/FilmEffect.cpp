#include "FilmEffect.h"

void FilmEffect::Init(unsigned width, unsigned height)
{
	int index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	_buffers[index]->AddDepthTarget();
	_buffers[index]->Init(width, height);

	index = int(_shaders.size());
	_shaders.push_back(Shader::Create());
	_shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_shaders[index]->LoadShaderPartFromFile("shaders/Post/film_frag.glsl", GL_FRAGMENT_SHADER);
	_shaders[index]->Link();
}

void FilmEffect::ApplyEffect(PostEffect* buffer)
{
	BindShader(0);
	_shaders[0]->SetUniform("u_Intensity", _intensity);
	_shaders[0]->SetUniform("u_Time", _time);
	buffer->BindColorAsTexture(0, 0, 0);
	_buffers[0]->RenderToFSQ();
	buffer->UnbindTexture(0);
	UnbindShader();
}

void FilmEffect::DrawToScreen()
{
	BindShader(0);
	_shaders[0]->SetUniform("u_Intensity", _intensity);
	_shaders[0]->SetUniform("u_Time", _time);

	BindColorAsTexture(0, 0, 0);
	_buffers[0]->DrawFullscreenQuad();

	UnbindTexture(0);
	UnbindShader();
}

float FilmEffect::GetIntensity() const
{
	return _intensity;
}

void FilmEffect::SetIntensity(float intensity)
{
	_intensity = intensity;
}

void FilmEffect::SetTime(float time)
{
	_time = time;
}
