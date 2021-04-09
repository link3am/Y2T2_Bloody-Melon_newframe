#include "Player.h"
#include "AudioEngine.h"

Player::Player(GameObject Obj)
{
	melonObj = &Obj;
	rotY = melonObj->get<Transform>().GetLocalRotation().y;
}


bool Player::PlayerMove(GLFWwindow* window, double dt, AudioEvent& se) {
	bool t = false;
	if (!death) {
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			melonObj->get<Transform>().MoveLocalFixed(0.07f, 0.0f, 0.0f);
			melonObj->get<Transform>().SetLocalRotation(melonObj->get<Transform>().GetLocalRotation().x, rotY+60, melonObj->get<Transform>().GetLocalRotation().z);
			faceingR = true;
			t = true;

		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {

			melonObj->get<Transform>().MoveLocalFixed(-0.07f, 0.0f, 0.0f);
			melonObj->get<Transform>().SetLocalRotation(melonObj->get<Transform>().GetLocalRotation().x, rotY + 300.0f, melonObj->get<Transform>().GetLocalRotation().z);
			//melonObj->get<Transform>().SetLocalScale(-scaleX, melonObj->get<Transform>().GetLocalScale().y, melonObj->get<Transform>().GetLocalScale().z);
			faceingR = false;  
			t = true;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

			if (isGround == true) {
				position += 10 * 0.0111;
				isGround = false;
				se.Play();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			if (isGround == true && melonObj->get<Transform>().GetLocalPosition().y >= 0.2)
			{
				melonObj->get<Transform>().MoveLocalFixed(0.0f, -0.2f, 0.0f);
				isGround = false;
				groundHight = 0.0f;
			}
		}
	}
	return t;
}
void Player::mapping()
{

	if (melonObj->get<Transform>().GetLocalPosition().x > 17.0f && melonObj->get<Transform>().GetLocalPosition().x < 23.0f && melonObj->get<Transform>().GetLocalPosition().y >= 3.9f)
	{//cutting board
		groundHight = 4.0f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 1.0f && melonObj->get<Transform>().GetLocalPosition().x < 3.0f && melonObj->get<Transform>().GetLocalPosition().y >= 1.9f)
	{//clock
		groundHight = 2.0f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 5.0f && melonObj->get<Transform>().GetLocalPosition().x < 11.0f && melonObj->get<Transform>().GetLocalPosition().y >= 2.9f)
	{//microwave
		groundHight = 3.0f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 24.0f && melonObj->get<Transform>().GetLocalPosition().x < 24.8f && melonObj->get<Transform>().GetLocalPosition().y >= 1.4f)
	{//can01
		groundHight = 1.5f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 26.5f && melonObj->get<Transform>().GetLocalPosition().x < 29.5f && melonObj->get<Transform>().GetLocalPosition().y >= 0.9f)
	{//dish
		groundHight = 1.0f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 30.6f && melonObj->get<Transform>().GetLocalPosition().x < 31.4f && melonObj->get<Transform>().GetLocalPosition().y >= 1.4f)
	{//can02
		groundHight = 1.5f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 35.7f && melonObj->get<Transform>().GetLocalPosition().x < 37.4f && melonObj->get<Transform>().GetLocalPosition().y >= 0.6f)
	{//pan01
		groundHight = 0.7f;
	}

	else if (melonObj->get<Transform>().GetLocalPosition().x > 72.5f && melonObj->get<Transform>().GetLocalPosition().x < 75.5f && melonObj->get<Transform>().GetLocalPosition().y >= 0.6f)//dish3
	{//dish02
		groundHight = 0.7f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 80.5f && melonObj->get<Transform>().GetLocalPosition().x < 83.5f && melonObj->get<Transform>().GetLocalPosition().y >= 0.6f)//dish5
	{//dish03
		groundHight = 0.7f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 87.5f && melonObj->get<Transform>().GetLocalPosition().x < 90.5f && melonObj->get<Transform>().GetLocalPosition().y >= 0.9f
		&& melonObj->get<Transform>().GetLocalPosition().y <= 1.7f)
	{//dish04 *4
		groundHight = 1.0f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 87.5f && melonObj->get<Transform>().GetLocalPosition().x < 90.5f && melonObj->get<Transform>().GetLocalPosition().y >= 1.9f)
	{//dish05 *4
		groundHight = 2.0f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 91.0f && melonObj->get<Transform>().GetLocalPosition().x < 97.0f && melonObj->get<Transform>().GetLocalPosition().y >= 2.9f)
	{//microwave
		groundHight = 3.0f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 97.6f && melonObj->get<Transform>().GetLocalPosition().x < 98.4f && melonObj->get<Transform>().GetLocalPosition().y >= 1.4f)
	{//can03 
		groundHight = 1.5f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 99.5f && melonObj->get<Transform>().GetLocalPosition().x < 103.5f && melonObj->get<Transform>().GetLocalPosition().y >= 4.5f)
	{//rice box
		groundHight = 4.6f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 106.5f && melonObj->get<Transform>().GetLocalPosition().x < 107.5f && melonObj->get<Transform>().GetLocalPosition().y >= 4.3f)
	{//wine01	
		groundHight = 4.4f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 112.0f && melonObj->get<Transform>().GetLocalPosition().x < 115.7f && melonObj->get<Transform>().GetLocalPosition().y >= 4.5f)
	{//wine - spon - wine
		groundHight = 4.6f;
	}

	else if (melonObj->get<Transform>().GetLocalPosition().x > 125.0f && melonObj->get<Transform>().GetLocalPosition().x < 133.0f && melonObj->get<Transform>().GetLocalPosition().y >= 1.4f)
	{//cans -kitchup
		groundHight = 1.5f;
	}
	else if (melonObj->get<Transform>().GetLocalPosition().x > 160.0f || melonObj->get<Transform>().GetLocalPosition().x < -3)
	{
		groundHight = -10.0f;
	}
	else
	{
		groundHight = 0;
	}

}

void Player::blocker()
{
	//microwave
	{
		if (melonObj->get<Transform>().GetLocalPosition().x > 5.0f && melonObj->get<Transform>().GetLocalPosition().x < 7.0f && melonObj->get<Transform>().GetLocalPosition().y < 2.9f)
		{
			melonObj->get<Transform>().MoveLocalFixed(-0.1f, 0.0f, 0.0f);
		}
		if (melonObj->get<Transform>().GetLocalPosition().x > 9.0f && melonObj->get<Transform>().GetLocalPosition().x < 11.5f && melonObj->get<Transform>().GetLocalPosition().y < 2.9f)
		{
			melonObj->get<Transform>().MoveLocalFixed(0.1f, 0.0f, 0.0f);
		}
	}
	//microwave
	{
		if (melonObj->get<Transform>().GetLocalPosition().x > 91.0f && melonObj->get<Transform>().GetLocalPosition().x < 93.0f && melonObj->get<Transform>().GetLocalPosition().y < 2.9f)
		{
			melonObj->get<Transform>().MoveLocalFixed(-0.1f, 0.0f, 0.0f);
		}
		if (melonObj->get<Transform>().GetLocalPosition().x > 95.0f && melonObj->get<Transform>().GetLocalPosition().x < 97.5f && melonObj->get<Transform>().GetLocalPosition().y < 2.9f)
		{
			melonObj->get<Transform>().MoveLocalFixed(0.1f, 0.0f, 0.0f);
		}
	}
	//pan
	if (melonObj->get<Transform>().GetLocalPosition().x > 35.7f && melonObj->get<Transform>().GetLocalPosition().x < 36.0f && melonObj->get<Transform>().GetLocalPosition().y < 0.6f)
	{
		melonObj->get<Transform>().MoveLocalFixed(-0.1f, 0.0f, 0.0f);
	}
	if (melonObj->get<Transform>().GetLocalPosition().x > 37.2f && melonObj->get<Transform>().GetLocalPosition().x < 37.4f && melonObj->get<Transform>().GetLocalPosition().y < 0.6f)
	{
		melonObj->get<Transform>().MoveLocalFixed(0.1f, 0.0f, 0.0f);
	}

	//kitchup
	if (melonObj->get<Transform>().GetLocalPosition().x > 130.9f && melonObj->get<Transform>().GetLocalPosition().x < 131.5f && melonObj->get<Transform>().GetLocalPosition().y < 1.4f)
	{
		melonObj->get<Transform>().MoveLocalFixed(-0.1f, 0.0f, 0.0f);
	}
	if (melonObj->get<Transform>().GetLocalPosition().x > 132.7f && melonObj->get<Transform>().GetLocalPosition().x < 133.0f && melonObj->get<Transform>().GetLocalPosition().y < 1.4f)
	{
		melonObj->get<Transform>().MoveLocalFixed(0.1f, 0.0f, 0.0f);
	}
}


void Player::PlayerPhy(GLFWwindow* window, double dt, AudioEvent& se)
{
	if (death == false) {
		if (PlayerMove(window, dt ,se)) {
			isPlayerMoving = true;
		}
		else {
			isPlayerMoving = false;
		}
		mapping();
		blocker();

		if (melonObj->get<Transform>().GetLocalPosition().y > groundHight)//gravity
		{
			//test 
			isGround = false;
			position += acceleration * 0.0111 * 0.0111;

		}

		if (melonObj->get<Transform>().GetLocalPosition().y < groundHight)
		{

			melonObj->get<Transform>().SetLocalPosition(melonObj->get<Transform>().GetLocalPosition().x, groundHight, melonObj->get<Transform>().GetLocalPosition().z);
			isGround = true;
			//test
			position = 0.0f;
		}

		melonObj->get<Transform>().MoveLocalFixed(0.0f, position, 0.0f);

		if (melonObj->get<Transform>().GetLocalPosition().y <= -5)
		{
			melonObj->get<Transform>().SetLocalPosition(0.0f, 0.5f, 0.0f);
			isGround = true;
		}
	}
}

bool Player::returnFace()
{ 
	if (melonObj->get<Transform>().GetLocalRotation().y < 180) {
		return true;
	}
	else {
		return false;
	}
}

glm::vec3 Player::getPosition()
{
	return melonObj->get<Transform>().GetLocalPosition();
}


glm::vec4  Player::getHitBox()
{
	return { melonObj->get<Transform>().GetLocalPosition().x, melonObj->get<Transform>().GetLocalPosition().y,0.8, 1 };
}