#pragma once
#include "Graphics/Post/PostEffect.h"

class BloomEffect : public PostEffect
{
public:
	
	void Init(unsigned width, unsigned height) override;

	
	void ApplyEffect(PostEffect* buffer) override;

	
	float GetThreshold() const;


	void SetThreshold(float intensity);
private:

	float _threshold = 0.5f;

};