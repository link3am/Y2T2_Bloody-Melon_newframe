#pragma once
#include "Utilities/BackendHandler.h"

class AudioEvent;
class Player
{
public:
	Player(GameObject Obj);

	bool PlayerMove(GLFWwindow* window, double dt, AudioEvent& se);
	void PlayerPhy(GLFWwindow* window, double dt, AudioEvent& se);
	void mapping();
	void blocker();
	bool returnFace();

	glm::vec4 getHitBox();
	glm::vec3 getPosition();

	bool isDeath() {
		return death;
	}

	void SetIsDeath(bool d) {
		death = d;
	}

	GameObject getPlayer() {
		return *melonObj;
	}

	bool getIsPlayerMoving() {
		return isPlayerMoving;
	}

private:
	GameObject* melonObj = nullptr;

	VertexArrayObject::sptr melonvao;

	float rotY = 0.0f;

	bool isPlayerMoving = false;
	float groundHight = 0.0f;
	bool isGround = true;
	bool faceingR = true;
	float acceleration = -25.0f;
	float position = 0.0f;

	bool canShoot = true;
	bool death = false;

};

