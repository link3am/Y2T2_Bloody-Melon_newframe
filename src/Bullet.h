#pragma once
#include "Utilities/BackendHandler.h"


class Bullet
{
public:
	Bullet(GameObject& Obj, bool isRight);

	void projectile();

	glm::vec4 getHitBox();

	bool isDeath() {
		return death;
	}

	void SetIsDeath(bool d) {
		death = d;
	}

	GameObject getBullet() {
		return bulletObj;
	}

	void update();
private:
	GameObject bulletObj;
	VertexArrayObject::sptr bulletvao;

	glm::vec3 startPos;
	glm::vec3 endPos;

	float t = 0;
	bool death = false;
	bool IsRight = true;
};

