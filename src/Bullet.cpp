#include "Bullet.h"
Bullet::Bullet(GameObject& Obj, bool isRight) :bulletObj(entt::handle(Obj.registry(), Obj.entity()))
{
	IsRight = isRight;

	if (IsRight) {
		startPos = glm::vec3(bulletObj.get<Transform>().GetLocalPosition().x + 0.3f, bulletObj.get<Transform>().GetLocalPosition().y, bulletObj.get<Transform>().GetLocalPosition().z-0.5);
	}
	else {
		startPos = glm::vec3(bulletObj.get<Transform>().GetLocalPosition().x - 0.3f, bulletObj.get<Transform>().GetLocalPosition().y, bulletObj.get<Transform>().GetLocalPosition().z);
	}

	if (IsRight) {
		endPos = glm::vec3(bulletObj.get<Transform>().GetLocalPosition().x + 10, bulletObj.get<Transform>().GetLocalPosition().y, bulletObj.get<Transform>().GetLocalPosition().z);
	}

	else {
		endPos = glm::vec3(bulletObj.get<Transform>().GetLocalPosition().x - 10, bulletObj.get<Transform>().GetLocalPosition().y, bulletObj.get<Transform>().GetLocalPosition().z);
	}
}

void Bullet::projectile()
{     
	t += .055;
	if (t > 1) {
		death = true;
	}
	glm::vec3 newPos = (endPos - startPos) * t + startPos;
	bulletObj.get<Transform>().SetLocalPosition(newPos.x, newPos.y, bulletObj.get<Transform>().GetLocalPosition().z);
}

glm::vec4 Bullet::getHitBox()
{
	return { bulletObj.get<Transform>().GetLocalPosition().x, bulletObj.get<Transform>().GetLocalPosition().y,1,1 };
}

void Bullet::update()
{
	projectile();
}


