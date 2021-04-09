#include "Enemy.h"

Enemy::Enemy(GameObject Obj)
{
	EnemyObj = &Obj;
	PatrolPoint1 = glm::vec3(EnemyObj->get<Transform>().GetLocalPosition().x - 1, EnemyObj->get<Transform>().GetLocalPosition().y, EnemyObj->get<Transform>().GetLocalPosition().z);
	PatrolPoint2 = glm::vec3(EnemyObj->get<Transform>().GetLocalPosition().x + 1, EnemyObj->get<Transform>().GetLocalPosition().y, EnemyObj->get<Transform>().GetLocalPosition().z);
}

void Enemy::AIPatrol()
{
	if (EnemyObj->get<Transform>().GetLocalScale().x > 0) {
		t += .005;
	}
	else {
		t -= .005;
	}

	if (t > 1) {
		EnemyObj->get<Transform>().SetLocalScale(-1.0f, 1.0f, -1.0f);
	}
	else if (t < 0)
	{
		EnemyObj->get<Transform>().SetLocalScale(1.0f, 1.0f, -1.0f);
	}

	glm::vec3 newPos = (PatrolPoint2 - PatrolPoint1) * t + PatrolPoint1;
	EnemyObj->get<Transform>().SetLocalPosition(newPos.x, newPos.y, EnemyObj->get<Transform>().GetLocalPosition().z);
}

void Enemy::setPatrolPoint(glm::vec3 p1, glm::vec3 p2)
{
	PatrolPoint1 = p1;
	PatrolPoint2 = p2;
}

glm::vec4  Enemy::getHitBox()
{
	return { EnemyObj->get<Transform>().GetLocalPosition().x, EnemyObj->get<Transform>().GetLocalPosition().y, 0.8,1};
}
