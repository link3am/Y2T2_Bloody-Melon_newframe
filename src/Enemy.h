#pragma once
#include "Utilities/BackendHandler.h"

class Enemy
{
public:
	Enemy(GameObject Obj);

	void AIPatrol();
	void setPatrolPoint(glm::vec3 p1, glm::vec3 p2);

	bool isDeath() {
		return death;
	}

	void SetIsDeath(bool d) {
		death = d;
	}

	GameObject getEnemy() {
		return *EnemyObj;
	}

	glm::vec4 getHitBox();
private:
	GameObject* EnemyObj = nullptr;

	VertexArrayObject::sptr Enemyvao;

	float t = 0;
	glm::vec3 PatrolPoint1 = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 PatrolPoint2 = glm::vec3(0.0, 0.0, 0.0);

	bool death = false;
};
