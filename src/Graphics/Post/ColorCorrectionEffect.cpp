#include "ColorCorrectionEffect.h"

void ColorCorrectionEffect::Init(unsigned width, unsigned height)
{
	int index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	_buffers[index]->AddDepthTarget();
	_buffers[index]->Init(width, height);

	//Setup shaders
	index = int(_shaders.size());
	_shaders.push_back(Shader::Create());
	_shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_shaders[index]->LoadShaderPartFromFile("shaders/color_correction_frag.glsl", GL_FRAGMENT_SHADER);
	_shaders[index]->Link();

	//Load in cube
	_Lut.loadFromFile("cubes/BrightenedCorrection.cube");

	PostEffect::Init(width, height);
}

void ColorCorrectionEffect::ApplyEffect(PostEffect* buffer)
{
	BindShader(0);
	buffer->BindColorAsTexture(0, 0, 0);
	_Lut.bind(30);

	_buffers[0]->RenderToFSQ();

	_Lut.unbind(30);
	buffer->UnbindTexture(0);
	UnbindShader();
}

LUT3D ColorCorrectionEffect::GetLUT() const
{
	return _Lut;
}

void ColorCorrectionEffect::SetLUT(LUT3D cube)
{
	_Lut = cube;
}