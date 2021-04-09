#pragma once

#include "Graphics/Post/PostEffect.h"

class FilmEffect : public PostEffect
{
public:
	void Init(unsigned width, unsigned height) override;

	void ApplyEffect(PostEffect* buffer) override;

	void DrawToScreen() override;

	float GetIntensity() const;

	void SetIntensity(float intensity);
	void SetTime(float time);
private:
	float _intensity = 1.0f;
	float _time = 0.0;

};