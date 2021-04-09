#include "GBuffer.h"

void GBuffer::Init(unsigned width, unsigned height)
{
	_windowWidth = width;
	_windowHeight = height;

	_gBuffer.AddColorTarget(GL_RGBA8);//Albeda Buffer
	_gBuffer.AddColorTarget(GL_RGB8); //Normals Buffer
	_gBuffer.AddColorTarget(GL_RGB8); //Specular Buffer

	_gBuffer.AddColorTarget(GL_RGB32F);

	//Add a depth buffer
	_gBuffer.AddDepthTarget();

	_gBuffer.Init(width, height);

	_passThrough = Shader::Create();
	_passThrough->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_passThrough->LoadShaderPartFromFile("shaders/passthrough_frag.glsl", GL_FRAGMENT_SHADER);
	_passThrough->Link();
}

void GBuffer::Bind()
{
	_gBuffer.Bind();
}

void GBuffer::BindLighting()
{
	_gBuffer.BindColorAsTexture(Target::ALBEDO, 0);
	_gBuffer.BindColorAsTexture(Target::NORMAL, 1);
	_gBuffer.BindColorAsTexture(Target::SPECULAR, 2);
	_gBuffer.BindColorAsTexture(Target::POSITION, 3);
}

void GBuffer::Clear()
{
	_gBuffer.Clear();
}

void GBuffer::Unbind()
{
	_gBuffer.Unbind();
}

void GBuffer::UnbindLighting()
{
	_gBuffer.UnbindTexture(0);
	_gBuffer.UnbindTexture(1);
	_gBuffer.UnbindTexture(2);
	_gBuffer.UnbindTexture(3);
}

void GBuffer::DrawBuffersToScreen()
{
	_passThrough->Bind();
	//top left
	glViewport(0, _windowHeight / 2.f, _windowWidth / 2.f, _windowHeight / 2.f);
	_gBuffer.BindColorAsTexture(Target::ALBEDO, 0);
	_gBuffer.DrawFullscreenQuad();
	_gBuffer.UnbindTexture(0);
	//top right
	glViewport(_windowWidth / 2.f, _windowHeight / 2.f, _windowWidth / 2.f, _windowHeight / 2.f);
	_gBuffer.BindColorAsTexture(Target::NORMAL,0);
	_gBuffer.DrawFullscreenQuad();
	_gBuffer.UnbindTexture(0);
	//bottom left
	glViewport(0, 0, _windowWidth / 2.f, _windowHeight / 2.f);
	_gBuffer.BindColorAsTexture(Target::SPECULAR, 0);
	_gBuffer.DrawFullscreenQuad();
	_gBuffer.UnbindTexture(0);
	//bottom right
	glViewport(_windowWidth / 2.f, 0, _windowWidth / 2.f, _windowHeight / 2.f);
	_gBuffer.BindColorAsTexture(Target::POSITION, 0);
	_gBuffer.DrawFullscreenQuad();
	_gBuffer.UnbindTexture(0);

	//Unbind our passtrough shader
	_passThrough->UnBind();
}

void GBuffer::PositionBuffer()
{
	_passThrough->Bind();
	_gBuffer.BindColorAsTexture(Target::POSITION, 0);
	_gBuffer.DrawFullscreenQuad();
	_gBuffer.UnbindTexture(0);
	_passThrough->UnBind();
}

void GBuffer::NormalBuffer()
{
	_passThrough->Bind();
	_gBuffer.BindColorAsTexture(Target::NORMAL, 0);
	_gBuffer.DrawFullscreenQuad();
	_gBuffer.UnbindTexture(0);
	_passThrough->UnBind();
}

void GBuffer::ColorBuffer()
{
	_passThrough->Bind();
	_gBuffer.BindColorAsTexture(Target::ALBEDO, 0);
	_gBuffer.DrawFullscreenQuad();
	_gBuffer.UnbindTexture(0);
	_passThrough->UnBind();
}

void GBuffer::Reshape(unsigned width, unsigned height)
{
	_windowWidth = width;
	_windowHeight = height;

	//Reshapes the framebuffer
	_gBuffer.Reshape(width, height);
}
