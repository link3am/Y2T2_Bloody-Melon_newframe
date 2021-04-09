#include "Utilities/BackendHandler.h"
#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <tiny_gltf.h>
#include <GLM/glm.hpp>

//TODO: New for this tutorial
#include <DirectionalLight.h>
#include <PointLight.h>
#include <UniformBuffer.h>
/////////////////////////////

#include <Texture2D.h>
#include <Texture2DData.h>
#include <MeshBuilder.h>
#include <MeshFactory.h>
#include <NotObjLoader.h>
#include <ObjLoader.h>
#include <VertexTypes.h>
#include <ShaderMaterial.h>

#include <RendererComponent.h>
#include <TextureCubeMap.h>
#include <TextureCubeMapData.h>

#include <Timing.h>
#include <GameObjectTag.h>
#include <InputHelpers.h>

#include <IBehaviour.h>
#include <CameraControlBehaviour.h>
#include <FollowPathBehaviour.h> 
#include <SimpleMoveBehaviour.h>
#include <Player.h>
#include <Bullet.h>
#include <Enemy.h>
#include <HitCheck.h>
#include <Graphics\GBuffer.h>
#include <Graphics\IlluminationBuffer.h>

//FMod
#include "AudioEngine.h"

#include "SkinnedMesh.h"

#include <stdlib.h> // srand  與 rand, C++ 為 cstdlib
#include <time.h>  // time(), C++ 為 ctime

using namespace std;

int main() {      
	int frameIx = 0;
	float fpsBuffer[128];
	float minFps, maxFps, avgFps;
	int selectedVao = 0; // select cube by default
	std::vector<GameObject> controllables;
	//key toggle
	//std::vector<GameObject> controllables;

	bool drawGBuffer = false;   
	bool drawIllumBuffer = false;  


	/// Fmod

	AudioEngine& engine = AudioEngine::Instance();
	engine.Init();
	engine.LoadBank("Master");
	AudioEvent& BGM = engine.CreateEvent1("Background Music", "{f165c66f-0811-48dd-82a9-b8d724e72698}");
	AudioEvent& SE_shoot = engine.CreateEvent1("Shut", "{0daa9b74-0dd4-416e-9865-5121600bb80d}");
	AudioEvent& SE_Jump = engine.CreateEvent1("Jump", "{d1d97e93-05c3-4482-8e04-0404dd8d2e7b}");
	AudioEvent& SE_MelonDie = engine.CreateEvent1("MelonDie", "{b217b8f4-43a7-43df-a7f3-0535437d4fb5}");
	AudioEvent& SE_Die = engine.CreateEvent1("Die", "{a1e9ee3d-3842-4e4d-804a-1fc307347bae}");

	BGM.Play();

	BackendHandler::InitAll();    

	glEnable(GL_DEBUG_OUTPUT);          
	glDebugMessageCallback(BackendHandler::GlDebugMessage, nullptr);
	glEnable(GL_TEXTURE_2D);     
	    
	GLTFSkinnedMesh main_mesh;   
	main_mesh.LoadFromFile("animation/walk.gltf");    
	main_mesh.SetLooping(true);     

	GLTFSkinnedMesh main_mesh2;
	main_mesh2.LoadFromFile("animation/idel.gltf");    
	main_mesh2.SetLooping(true);

	GLTFSkinnedMesh knife_mesh;
	knife_mesh.LoadFromFile("animation/knife.gltf");
	knife_mesh.SetLooping(true);

#pragma region Shader and ImGui 
	Shader::sptr AnimShader = Shader::Create();
	AnimShader->LoadShaderPartFromFile("shaders/skinned_vertex_shader.glsl", GL_VERTEX_SHADER);
	AnimShader->LoadShaderPartFromFile("shaders/gBuffer_pass_frag.glsl", GL_FRAGMENT_SHADER);
	AnimShader->Link();

	Shader::sptr passthroughShader = Shader::Create();
	passthroughShader->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	passthroughShader->LoadShaderPartFromFile("shaders/passthrough_frag.glsl", GL_FRAGMENT_SHADER);
	passthroughShader->Link();  

	Shader::sptr simpleDepthShader = Shader::Create();
	simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_vert.glsl", GL_VERTEX_SHADER);
	simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_frag.glsl", GL_FRAGMENT_SHADER);
	simpleDepthShader->Link();
	 
	//Init gBuffer shader
	Shader::sptr gBufferShader = Shader::Create();
	gBufferShader->LoadShaderPartFromFile("Shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
	gBufferShader->LoadShaderPartFromFile("Shaders/gBuffer_pass_frag.glsl", GL_FRAGMENT_SHADER); 
	gBufferShader->Link();

	Shader::sptr shader = Shader::Create();
	shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
	//Directional Light Shader
	shader->LoadShaderPartFromFile("shaders/directional_blinn_phong_frag.glsl", GL_FRAGMENT_SHADER);
	shader->Link();


	//Creates our directional Light
	//DirectionalLight theSun;
	//UniformBuffer directionalLightBuffer;

	PostEffect* basicEffect;
	Framebuffer* shadowBuffer;
	GBuffer* gBuffer;
	IlluminationBuffer* illuminationBuffer;

	int activeEffect = 0;

	int toggleScene = 0;

	std::vector<PostEffect*> effects;
	SepiaEffect* sepiaEffect;
	GreyscaleEffect* greyscaleEffect;
	PixelEffect* pixelEffect;
	BloomEffect* bloomEffect;
	FilmEffect* filmEffect;

	//Allocates enough memory for one directional light (we can change this easily, but we only need 1 directional light)
	//directionalLightBuffer.AllocateMemory(sizeof(DirectionalLight));
	//Casts our sun as "data" and sends it to the shader
	//directionalLightBuffer.SendData(reinterpret_cast<void*>(&theSun), sizeof(DirectionalLight));

	//directionalLightBuffer.Bind(0);


	glm::vec3 lightPos = glm::vec3(0.0f, 2.0f, 0.0f);
	glm::vec3 lightCol = glm::vec3(0.9f, 0.85f, 0.5f);
	float     lightAmbientPow = 0.05f;
	float     lightSpecularPow = 1.0f;
	glm::vec3 ambientCol = glm::vec3(1.0f);
	float     ambientPow = 0.1f;
	float     lightLinearFalloff = 0.09f;
	float     lightQuadraticFalloff = 0.032f;
	bool CelShading = false;
	bool Normal = false;

	shader->SetUniform("u_LightPos", lightPos);
	shader->SetUniform("u_LightCol", lightCol);
	shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
	shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
	shader->SetUniform("u_AmbientCol", ambientCol);
	shader->SetUniform("u_AmbientStrength", ambientPow);
	shader->SetUniform("u_LightAttenuationConstant", 1.0f);
	shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
	shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);
	shader->SetUniform("u_CelShading", (int)CelShading);
	shader->SetUniform("u_Normal", (int)Normal);

	AnimShader->SetUniform("u_LightPos", lightPos);
	AnimShader->SetUniform("u_LightCol", lightCol);
	AnimShader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
	AnimShader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
	AnimShader->SetUniform("u_AmbientCol", ambientCol);
	AnimShader->SetUniform("u_AmbientStrength", ambientPow);

	//3am
	//BloomEffect* bloomEffect;


	vector<Bullet> BulletList;
	vector<Enemy> EnemyList;
	vector<GameObject>EnemyObjList;
	vector<GameObject> BulletObjList;

#pragma endregion

#pragma region ImGui 
	BackendHandler::imGuiCallbacks.push_back([&]() {
		if (ImGui::CollapsingHeader("toggles "))
		{
			ImGui::SliderInt("Chosen Effect", &activeEffect, 0, effects.size() - 1);

			if (activeEffect == 0)
			{
				ImGui::Text("Active Effect: basic Effect");

				PostEffect* temp = (PostEffect*)effects[activeEffect];
			}
			if (activeEffect == 1)
			{
				ImGui::Text("Active Effect: Sepia Effect");

				SepiaEffect* temp = (SepiaEffect*)effects[activeEffect];
				float intensity = temp->GetIntensity();

				if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f))
				{
					temp->SetIntensity(intensity);
				}
			}
			if (activeEffect == 2) 
			{
				ImGui::Text("Active Effect: Greyscale Effect");

				GreyscaleEffect* temp = (GreyscaleEffect*)effects[activeEffect];
				float intensity = temp->GetIntensity();

				if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f))
				{
					temp->SetIntensity(intensity);
				}
			}
			if (activeEffect == 3)
			{
				ImGui::Text("Active Effect: Pixel Effect");

				PixelEffect* temp = (PixelEffect*)effects[activeEffect];
				float intensity = temp->GetIntensity();

				if (ImGui::SliderFloat("Intensity", &intensity, 0.001f, 0.01f))
				{
					temp->SetIntensity(intensity);
				}
			}
			if (activeEffect == 4)
			{
				ImGui::Text("Active Effect: bloom Effect");

				BloomEffect* temp = (BloomEffect*)effects[activeEffect];
				float intensity = temp->GetThreshold();

				if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f))
				{
					temp->SetThreshold(intensity);
				}
			}
			if (activeEffect == 5)
			{
				ImGui::Text("Active Effect: Film Effect");

				FilmEffect* temp = (FilmEffect*)effects[activeEffect];
				float intensity = temp->GetIntensity();

				if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f))
				{
					temp->SetIntensity(intensity);
				}
			}
		}

		if (ImGui::CollapsingHeader("Switch Scene"))
		{
			ImGui::SliderInt("toggleScene", &toggleScene, 0, 4);
			 
			if (toggleScene == 0)
			{
				ImGui::Text(" deferred light source");
				drawGBuffer = false;
				effects[activeEffect]->ApplyEffect(illuminationBuffer);
				effects[activeEffect]->DrawToScreen();
			} 
			if (toggleScene == 1)
			{
				ImGui::Text(" Position Buffer");
				gBuffer->PositionBuffer();
				drawGBuffer = false; 
			}
			if (toggleScene == 2) 
			{
				ImGui::Text(" Normal Buffer");
				gBuffer->NormalBuffer(); 
				drawGBuffer = false;
			}
			if (toggleScene == 3) 
			{
				ImGui::Text(" Color Buffer");
				gBuffer->ColorBuffer();
				drawGBuffer = false;
			}
			if (toggleScene == 4)
			{
				ImGui::Text(" Color Buffer");
				illuminationBuffer->DrawIllumBuffer();
				drawGBuffer = false;
			} 
		}       
		 

		if (ImGui::CollapsingHeader("Light Level Lighting Settings"))
		{
			if (ImGui::DragFloat3("Light Direction/Position", glm::value_ptr(illuminationBuffer->GetSunRef()._lightDirection), 0.01f, -10.0f, 10.0f))
			{
				//directionalLightBuffer.SendData(reinterpret_cast<void*>(&illuminationBuffer->GetSunRef()), sizeof(DirectionalLight));
			}
			if (ImGui::ColorPicker3("Light Col", glm::value_ptr(illuminationBuffer->GetSunRef()._lightCol)))
			{
				//directionalLightBuffer.SendData(reinterpret_cast<void*>(&illuminationBuffer->GetSunRef()), sizeof(DirectionalLight));
			}
		}
		 

		ImGui::Text("Q/E -> Yaw\nLeft/Right -> Roll\nUp/Down -> Pitch\nY -> Toggle Mode");

		minFps = FLT_MAX;
		maxFps = 0;
		avgFps = 0;
		for (int ix = 0; ix < 128; ix++) {
			if (fpsBuffer[ix] < minFps) { minFps = fpsBuffer[ix]; }
			if (fpsBuffer[ix] > maxFps) { maxFps = fpsBuffer[ix]; }
			avgFps += fpsBuffer[ix];
		}
		ImGui::PlotLines("FPS", fpsBuffer, 128);
		ImGui::Text("MIN: %f MAX: %f AVG: %f", minFps, maxFps, avgFps / 128.0f);
		});

#pragma endregion
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL); // New     

	// Load some textures from files
	Texture2D::sptr noSpec = Texture2D::LoadFromFile("images/grassSpec.png");
	TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/ToonSky.jpg");
	//melon
	Texture2D::sptr melonUV = Texture2D::LoadFromFile("images/melon UV.png");
	//stuff       
	Texture2D::sptr tableUV = Texture2D::LoadFromFile("images/table UV.png");
	Texture2D::sptr knifeUV = Texture2D::LoadFromFile("images/knife UV.png");


	Texture2D::sptr titleUV = Texture2D::LoadFromFile("images/title UV.png");
	Texture2D::sptr WinUV = Texture2D::LoadFromFile("images/win UV.png");
	Texture2D::sptr loseUV = Texture2D::LoadFromFile("images/lose UV.png");


	Texture2D::sptr seedsTUV = Texture2D::LoadFromFile("images/seedsT UV.png");
	Texture2D::sptr seedsUV = Texture2D::LoadFromFile("images/seeds UV.png");

	Texture2D::sptr mapUV = Texture2D::LoadFromFile("images/lv_1.png");

	Texture2D::sptr uv1 = Texture2D::LoadFromFile("images/clock UV.png");
	Texture2D::sptr uv2 = Texture2D::LoadFromFile("images/cutting UV.png");
	Texture2D::sptr uv3 = Texture2D::LoadFromFile("images/microwave UV.png");     
	Texture2D::sptr uv4 = Texture2D::LoadFromFile("images/can UV.png");
	Texture2D::sptr uv5 = Texture2D::LoadFromFile("images/dish UV.png");
	Texture2D::sptr uv6 = Texture2D::LoadFromFile("images/pan UV.png");
	Texture2D::sptr uv7 = Texture2D::LoadFromFile("images/pot UV.png");
	Texture2D::sptr uv8 = Texture2D::LoadFromFile("images/kicha UV.png");
	Texture2D::sptr uv9 = Texture2D::LoadFromFile("images/wine UV.png");
	Texture2D::sptr uv10 = Texture2D::LoadFromFile("images/donut UV.png");
	Texture2D::sptr uv11 = Texture2D::LoadFromFile("images/chess UV.png");

	///////////////////////////////////// Scene Generation //////////////////////////////////////////////////
#pragma region Scene Generation

// We need to tell our scene system what extra component types we want to support
	GameScene::RegisterComponentType<RendererComponent>();
	GameScene::RegisterComponentType<BehaviourBinding>();
	GameScene::RegisterComponentType<Camera>();

	// Create a scene, and set it to be the active scene in the application
	GameScene::sptr scene = GameScene::Create("scene 1");
	Application::Instance().ActiveScene = scene;

	// We can create a group ahead of time to make iterating on the group faster
	entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup
		= scene->Registry().group<RendererComponent>(entt::get_t<Transform>());

	// Create a material and set some properties for it


	ShaderMaterial::sptr melonMat = ShaderMaterial::Create();
	melonMat->Shader = gBufferShader;
	melonMat->Set("s_Diffuse", melonUV);
	melonMat->Set("s_Specular", noSpec);
	melonMat->Set("u_Shininess", 8.0f);
	melonMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr AnimMelonMat = ShaderMaterial::Create();
	AnimMelonMat->Shader = AnimShader;
	AnimMelonMat->Set("s_Diffuse", melonUV);
	AnimMelonMat->Set("s_Specular", noSpec);
	AnimMelonMat->Set("u_Shininess", 8.0f);
	AnimMelonMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr AnimKnifeMat = ShaderMaterial::Create();
	AnimKnifeMat->Shader = AnimShader;
	AnimKnifeMat->Set("s_Diffuse", knifeUV);
	AnimKnifeMat->Set("s_Specular", noSpec);
	AnimKnifeMat->Set("u_Shininess", 8.0f);
	AnimKnifeMat->Set("u_TextureMix", 0.0f);


	ShaderMaterial::sptr knifeMat = ShaderMaterial::Create();
	knifeMat->Shader = gBufferShader;
	knifeMat->Set("s_Diffuse", knifeUV);
	knifeMat->Set("s_Specular", noSpec);
	knifeMat->Set("u_Shininess", 8.0f);
	knifeMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr titleMat = ShaderMaterial::Create();
	titleMat->Shader = gBufferShader;
	titleMat->Set("s_Diffuse", titleUV);
	titleMat->Set("s_Specular", noSpec);
	titleMat->Set("u_Shininess", 8.0f);
	titleMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr WinMat = ShaderMaterial::Create();
	WinMat->Shader = gBufferShader;
	WinMat->Set("s_Diffuse", WinUV);
	WinMat->Set("s_Specular", noSpec);
	WinMat->Set("u_Shininess", 8.0f);
	WinMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr loseMat = ShaderMaterial::Create();
	loseMat->Shader = gBufferShader;
	loseMat->Set("s_Diffuse", loseUV);
	loseMat->Set("s_Specular", noSpec);
	loseMat->Set("u_Shininess", 8.0f);
	loseMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr seedsTMat = ShaderMaterial::Create();
	seedsTMat->Shader = gBufferShader;
	seedsTMat->Set("s_Diffuse", seedsTUV);
	seedsTMat->Set("s_Specular", noSpec);
	seedsTMat->Set("u_Shininess", 8.0f);
	seedsTMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr tableMat = ShaderMaterial::Create();
	tableMat->Shader = gBufferShader;
	tableMat->Set("s_Diffuse", tableUV);
	tableMat->Set("s_Specular", noSpec);
	tableMat->Set("u_Shininess", 8.0f);
	tableMat->Set("u_TextureMix", 0.0f);
	//stuff material

	ShaderMaterial::sptr clockMat = ShaderMaterial::Create();
	clockMat->Shader = gBufferShader;
	clockMat->Set("s_Diffuse", uv1);
	clockMat->Set("s_Specular", noSpec);
	clockMat->Set("u_Shininess", 8.0f);
	clockMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr cuttingMat = ShaderMaterial::Create();
	cuttingMat->Shader = gBufferShader;
	cuttingMat->Set("s_Diffuse", uv2);
	cuttingMat->Set("s_Specular", noSpec);
	cuttingMat->Set("u_Shininess", 8.0f);
	cuttingMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr microwaveMat = ShaderMaterial::Create();
	microwaveMat->Shader = gBufferShader;
	microwaveMat->Set("s_Diffuse", uv3);
	microwaveMat->Set("s_Specular", noSpec);
	microwaveMat->Set("u_Shininess", 8.0f);
	microwaveMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr canMat = ShaderMaterial::Create();
	canMat->Shader = gBufferShader;
	canMat->Set("s_Diffuse", uv4);
	canMat->Set("s_Specular", noSpec);
	canMat->Set("u_Shininess", 8.0f);
	canMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr dishMat = ShaderMaterial::Create();
	dishMat->Shader = gBufferShader;
	dishMat->Set("s_Diffuse", uv5);
	dishMat->Set("s_Specular", noSpec);
	dishMat->Set("u_Shininess", 8.0f);
	dishMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr panMat = ShaderMaterial::Create();
	panMat->Shader = gBufferShader;
	panMat->Set("s_Diffuse", uv6); 
	panMat->Set("s_Specular", noSpec);
	panMat->Set("u_Shininess", 8.0f);
	panMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr seedsMat = ShaderMaterial::Create();
	seedsMat->Shader = gBufferShader;
	seedsMat->Set("s_Diffuse", seedsUV);
	seedsMat->Set("s_Specular", noSpec);
	seedsMat->Set("u_Shininess", 8.0f);
	seedsMat->Set("u_TextureMix", 0.0f);     

	ShaderMaterial::sptr potMat = ShaderMaterial::Create();
	potMat->Shader = gBufferShader;
	potMat->Set("s_Diffuse", uv7);
	potMat->Set("s_Specular", noSpec);
	potMat->Set("u_Shininess", 8.0f);
	potMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr kichaMat = ShaderMaterial::Create();
	kichaMat->Shader = gBufferShader;
	kichaMat->Set("s_Diffuse", uv8);
	kichaMat->Set("s_Specular", noSpec);
	kichaMat->Set("u_Shininess", 8.0f);
	kichaMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr wineMat = ShaderMaterial::Create();
	wineMat->Shader = gBufferShader;
	wineMat->Set("s_Diffuse", uv9);
	wineMat->Set("s_Specular", noSpec);
	wineMat->Set("u_Shininess", 8.0f);
	wineMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr donutMat = ShaderMaterial::Create();
	donutMat->Shader = gBufferShader;
	donutMat->Set("s_Diffuse", uv10);
	donutMat->Set("s_Specular", noSpec);
	donutMat->Set("u_Shininess", 8.0f);
	donutMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr chessMat = ShaderMaterial::Create();
	chessMat->Shader = gBufferShader;
	chessMat->Set("s_Diffuse", uv11);
	chessMat->Set("s_Specular", noSpec);
	chessMat->Set("u_Shininess", 8.0f);
	chessMat->Set("u_TextureMix", 0.0f);

	ShaderMaterial::sptr mapMat = ShaderMaterial::Create();
	mapMat->Shader = gBufferShader;
	mapMat->Set("s_Diffuse", mapUV);
	mapMat->Set("s_Specular", noSpec);
	mapMat->Set("u_Shininess", 8.0f);
	mapMat->Set("u_TextureMix", 0.0f);

	VertexArrayObject::sptr melonvao = ObjLoader::LoadFromFile("models/watermelon.obj");
	VertexArrayObject::sptr seedsvao = ObjLoader::LoadFromFile("models/seeds.obj");
	VertexArrayObject::sptr knifevao = ObjLoader::LoadFromFile("models/knife.obj");
	VertexArrayObject::sptr mapvao = ObjLoader::LoadFromFile("models/lv_1.obj");

	GameObject melonObj = scene->CreateEntity("melon"); 
	{
		melonObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);  
		melonObj.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
		melonObj.get<Transform>().SetLocalScale(0.5f, 0.5f, 0.5f);  
	}            
	Player P1 = Player(melonObj);

	  
	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy.get<Transform>().SetLocalPosition(15.0f, 0.0f, 0.0f);
		enemy.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E1 = Enemy(enemy);
	EnemyList.push_back(E1);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy2 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy2.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy2.get<Transform>().SetLocalPosition(6.0f, 2.9f, 0.0f);
		enemy2.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E2 = Enemy(enemy2);
	EnemyList.push_back(E2);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy3 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy3.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy3.get<Transform>().SetLocalPosition(30.0f, 0.0f, 0.0f);
		enemy3.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E3 = Enemy(enemy3);
	EnemyList.push_back(E3);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy4 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy4.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy4.get<Transform>().SetLocalPosition(21.0f, 4.0f, -1.2f);
		enemy4.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E4 = Enemy(enemy4);
	EnemyList.push_back(E4);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy5 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy5.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy5.get<Transform>().SetLocalPosition(40.0f, 0.0f, 0.0f);
		enemy5.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E5 = Enemy(enemy5);
	EnemyList.push_back(E5);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy6 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy6.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy6.get<Transform>().SetLocalPosition(44.0f, 0.0f, 0.0f);
		enemy6.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E6 = Enemy(enemy6);
	EnemyList.push_back(E6);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy7 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy7.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy7.get<Transform>().SetLocalPosition(50.0f, 0.0f, 0.0f);
		enemy7.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E7 = Enemy(enemy7);
	EnemyList.push_back(E7);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy8 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy8.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy8.get<Transform>().SetLocalPosition(57.0f, 0.0f, 0.0f);
		enemy8.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E8 = Enemy(enemy8);
	EnemyList.push_back(E8);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy9 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy9.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy9.get<Transform>().SetLocalPosition(67.4f, 0.0f, 0.0f);
		enemy9.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E9 = Enemy(enemy9);
	EnemyList.push_back(E9);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy10 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy10.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy10.get<Transform>().SetLocalPosition(73.7f, 0.0f, 0.0f);
		enemy10.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E10 = Enemy(enemy10);
	EnemyList.push_back(E10);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy11 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy11.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy11.get<Transform>().SetLocalPosition(93.9f, 3.0f, 0.0f);
		enemy11.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E11 = Enemy(enemy11);
	EnemyList.push_back(E11);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy12 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy12.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy12.get<Transform>().SetLocalPosition(101.5f, 4.6f, 0.0f);
		enemy12.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E12 = Enemy(enemy12);
	EnemyList.push_back(E12);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy13 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy13.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy13.get<Transform>().SetLocalPosition(113.96f, 4.6f, 0.0f);
		enemy13.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E13 = Enemy(enemy13);
	EnemyList.push_back(E13);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy14 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy14.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy14.get<Transform>().SetLocalPosition(125.0f, 0.0f, 0.0f);
		enemy14.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E14 = Enemy(enemy14);
	EnemyList.push_back(E14);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy15 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy15.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy15.get<Transform>().SetLocalPosition(140.0f, 0.0f, 0.0f);
		enemy15.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E15 = Enemy(enemy15);
	EnemyList.push_back(E15);

	EnemyObjList.push_back(scene->CreateEntity("enemy"));
	GameObject& enemy16 = EnemyObjList[EnemyObjList.size() - 1];
	{
		enemy16.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
		enemy16.get<Transform>().SetLocalPosition(154.63f, 0.0f, 0.0f);
		enemy16.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}
	Enemy E16 = Enemy(enemy16);
	EnemyList.push_back(E16);

	VertexArrayObject::sptr titlevao = ObjLoader::LoadFromFile("models/title.obj");
	GameObject title = scene->CreateEntity("title");
	{

		title.emplace<RendererComponent>().SetMesh(titlevao).SetMaterial(titleMat);
		title.get<Transform>().SetLocalPosition(-3.5f, -1.0f, 2.5f);
		title.get<Transform>().SetLocalScale(0.6f, 1.1f, 1.0f);
		title.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
	}

	GameObject seedsT = scene->CreateEntity("seedsT");
	{
		VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/seedsT.obj");
		seedsT.emplace<RendererComponent>().SetMesh(vao).SetMaterial(seedsTMat);
		seedsT.get<Transform>().SetLocalPosition(-1.8f, 1.7f, 2.5f);
		seedsT.get<Transform>().SetLocalScale(0.2f, 1.1f, 0.9f);
		seedsT.get<Transform>().SetLocalRotation(0.0f, 90.0f, 30.0f);
	}

	GameObject seedsT2 = scene->CreateEntity("seedsT");
	{
		VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/seedsT.obj");
		seedsT2.emplace<RendererComponent>().SetMesh(vao).SetMaterial(seedsTMat);
		seedsT2.get<Transform>().SetLocalPosition(2.0f, 1.5f, 2.5f);
		seedsT2.get<Transform>().SetLocalScale(0.2f, 1.1f, 0.9f);
		seedsT2.get<Transform>().SetLocalRotation(0.0f, 90.0f, 160.0f);
	}

	GameObject seedsT3 = scene->CreateEntity("seedsT");
	{
		VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/seedsT.obj");
		seedsT3.emplace<RendererComponent>().SetMesh(vao).SetMaterial(seedsTMat);
		seedsT3.get<Transform>().SetLocalPosition(0.3f, 1.5f, 2.5f);
		seedsT3.get<Transform>().SetLocalScale(0.1f, 1.1f, 0.9f);
		seedsT3.get<Transform>().SetLocalRotation(0.0f, 90.0f, 340.0f);
	}

	GameObject MaoObj = scene->CreateEntity("lv_1");
	{
		MaoObj.emplace<RendererComponent>().SetMesh(mapvao).SetMaterial(mapMat);
		MaoObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
		MaoObj.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
		MaoObj.get<Transform>().SetLocalScale(1.0f, 1.0f, 1.0f);
	}

	// Create an object to be our camera
	GameObject cameraObject = scene->CreateEntity("Camera");
	{
		cameraObject.get<Transform>().SetLocalPosition(0, 2.5, 6).LookAt(glm::vec3(0, 1, 0)).SetLocalRotation(glm::vec3(0, 0, 0));

		// We'll make our camera a component of the camera object
		Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
		camera.SetPosition(glm::vec3(0, 2.5, 6));
		camera.SetUp(glm::vec3(0, 1, 0));
		camera.LookAt(glm::vec3(0, 1, 0));
		camera.SetFovDegrees(90.0f); // Set an initial FOV
		camera.SetOrthoHeight(3.0f);
		//BehaviourBinding::Bind<CameraControlBehaviour>(cameraObject);
	}



	int width, height;
	glfwGetWindowSize(BackendHandler::window, &width, &height);

	GameObject gBufferObject = scene->CreateEntity("G Buffer");
	{
		gBuffer = &gBufferObject.emplace<GBuffer>();
		gBuffer->Init(width, height);
	}

	GameObject illuminationbufferObject = scene->CreateEntity("Illumination Buffer");
	{
		illuminationBuffer = &illuminationbufferObject.emplace<IlluminationBuffer>();
		illuminationBuffer->Init(width, height);
	}

	int shadowWitch = 4096;
	int shadowHeight = 4096;

	GameObject shadowBufferObject = scene->CreateEntity("Shadow Buffer");
	{
		shadowBuffer = &shadowBufferObject.emplace<Framebuffer>();
		shadowBuffer->AddDepthTarget();
		shadowBuffer->Init(shadowWitch, shadowHeight);
	}

	GameObject framebufferObject = scene->CreateEntity("Basic Effect");
	{
		basicEffect = &framebufferObject.emplace<PostEffect>();
		basicEffect->Init(width, height);
	}
	effects.push_back(basicEffect);

	GameObject sepiaEffectObject = scene->CreateEntity("Sepia Effect");
	{
		sepiaEffect = &sepiaEffectObject.emplace<SepiaEffect>();
		sepiaEffect->Init(width, height);
	}
	effects.push_back(sepiaEffect);

	GameObject greyscaleEffectObject = scene->CreateEntity("Greyscale Effect");
	{
		greyscaleEffect = &greyscaleEffectObject.emplace<GreyscaleEffect>();
		greyscaleEffect->Init(width, height);
	}
	effects.push_back(greyscaleEffect);

	GameObject pixelEffectObject = scene->CreateEntity("Pixel Effect");
	{
		pixelEffect = &pixelEffectObject.emplace<PixelEffect>();
		pixelEffect->Init(width, height);
	}
	effects.push_back(pixelEffect);

	GameObject bloomEffectObject = scene->CreateEntity("Bloom Effect");
	{
		bloomEffect = &bloomEffectObject.emplace<BloomEffect>();
		bloomEffect->Init(width, height);
	}
	effects.push_back(bloomEffect);

	GameObject filmEffectObject = scene->CreateEntity("Film Effect");
	{
		filmEffect = &filmEffectObject.emplace<FilmEffect>();
		filmEffect->Init(width, height);
	}
	effects.push_back(filmEffect);
#pragma endregion 
	//////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////// SKYBOX ///////////////////////////////////////////////

		// Load our shaders
	Shader::sptr skybox = std::make_shared<Shader>();
	skybox->LoadShaderPartFromFile("shaders/skybox-shader.vert.glsl", GL_VERTEX_SHADER);
	skybox->LoadShaderPartFromFile("shaders/skybox-shader.frag.glsl", GL_FRAGMENT_SHADER);
	skybox->Link();
	 
	ShaderMaterial::sptr skyboxMat = ShaderMaterial::Create();
	skyboxMat->Shader = skybox;
	skyboxMat->Set("s_Environment", environmentMap);
	skyboxMat->Set("u_EnvironmentRotation", glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))));
	skyboxMat->RenderLayer = 100;

	MeshBuilder<VertexPosNormTexCol> mesh;
	MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
	MeshFactory::InvertFaces(mesh);
	VertexArrayObject::sptr meshVao = mesh.Bake();

	GameObject skyboxObj = scene->CreateEntity("skybox");
	skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
	//skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat);

	int min, max=0;
	float CamMoveX=0;
	float CamMoveY=0;


// We'll use a vector to store all our key press events for now (this should probably be a behaviour eventually)
	std::vector<KeyPressWatcher> keyToggles;
	{
		// This is an example of a key press handling helper. Look at InputHelpers.h an .cpp to see
		// how this is implemented. Note that the ampersand here is capturing the variables within
		// the scope. If you wanted to do some method on the class, your best bet would be to give it a method and
		// use std::bind
		keyToggles.emplace_back(GLFW_KEY_T, [&]() { cameraObject.get<Camera>().ToggleOrtho(); });

		//Toggles drawing
		keyToggles.emplace_back(GLFW_KEY_F1, [&]() {drawGBuffer = !drawGBuffer; });
		keyToggles.emplace_back(GLFW_KEY_F2, [&]() {drawIllumBuffer = !drawIllumBuffer; });
		controllables.push_back(melonObj);
		 
		keyToggles.emplace_back(GLFW_KEY_KP_ADD, [&]() {
			BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
			selectedVao++;
			if (selectedVao >= controllables.size())
				selectedVao = 0;
			BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
			});
		keyToggles.emplace_back(GLFW_KEY_KP_SUBTRACT, [&]() {
			BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
			selectedVao--;
			if (selectedVao < 0)
				selectedVao = controllables.size() - 1;
			BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
			});

		keyToggles.emplace_back(GLFW_KEY_Y, [&]() {
			auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
			behaviour->Relative = !behaviour->Relative;
			});
	}


	// time
	Timing& time = Timing::Instance();
	time.LastFrame = glfwGetTime();
	double lastfpsTime = glfwGetTime();//60hz

	//timer for bullet

	Timing& Btime = Timing::Instance();
	Btime.LastFrame = glfwGetTime();
	double BlastfpsTime = glfwGetTime();//60hz

	int sceneNumber  = 0; //0= start menu, -1 = pause , 1= level 1
	int option = 0;

	int Animt = 0;
	double asTimer = 0;

	///// Game loop /////
	while (!glfwWindowShouldClose(BackendHandler::window)) {
		engine.Update();
		glfwPollEvents();
		// time,FPS, etc

		time.CurrentFrame = glfwGetTime();
		time.DeltaTime = static_cast<float>(time.CurrentFrame - time.LastFrame);
		time.DeltaTime = time.DeltaTime > 1.0f ? 1.0f : time.DeltaTime;

		Btime.CurrentFrame = glfwGetTime();
		Btime.DeltaTime = static_cast<float>(Btime.CurrentFrame - Btime.LastFrame);
		Btime.DeltaTime = Btime.DeltaTime > 1.0f ? 1.0f : Btime.DeltaTime;

		fpsBuffer[frameIx] = 1.0f / time.DeltaTime;
		fpsBuffer[frameIx+1] = 1.0f / Btime.DeltaTime;

		frameIx++;
		if (frameIx >= 128)
			frameIx = 0;

		if (sceneNumber == 1) {
			// We'll make sure our UI isn't focused before we start handling input for our game
			if (!ImGui::IsAnyWindowFocused()) {
				// We need to poll our key watchers so they can do their logic with the GLFW state
				// Note that since we want to make sure we don't copy our key handlers, we need a const
				// reference!
				for (const KeyPressWatcher& watcher : keyToggles) {
					watcher.Poll(BackendHandler::window);
				}
			}



			for (int i = 0; i < BulletList.size(); i++) {
				BulletList[i].projectile();

				if (BulletList[i].isDeath()) {
					scene->RemoveEntity(BulletList[i].getBullet());
					BulletList.erase(BulletList.begin() + i);
					BulletObjList.erase(BulletObjList.begin() + i);
					i--;
				}
			}



			if (P1.isDeath()) {

				if (P1.getPosition().x < 159) {
					title.get<RendererComponent>().SetMesh(titlevao).SetMaterial(loseMat);
				}
				else {
					title.get<RendererComponent>().SetMesh(titlevao).SetMaterial(WinMat);
					BGM.SetParameter("Loop 3", 1);
					BGM.SetParameter("Loop 2", 0);
				}
				title.get<Transform>().SetLocalPosition(P1.getPosition().x - 3.5f, P1.getPosition().y - 1.0f, 2.5f);
				title.get<Transform>().SetLocalScale(0.6f, 1.1f, 1.0f);
				title.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
				melonObj.get<Transform>().SetLocalPosition(melonObj.get<Transform>().GetLocalPosition().x, melonObj.get<Transform>().GetLocalPosition().y, -10.0f);
				
				if (glfwGetKey(BackendHandler::window, GLFW_KEY_ENTER) == GLFW_PRESS&& P1.isDeath()) {
					melonObj.get<Transform>().SetLocalPosition(0.0f, 0.5f, 0.0f);
					P1.SetIsDeath(false);
					title.get<Transform>().SetLocalPosition(title.get<Transform>().GetLocalPosition().x, title.get<Transform>().GetLocalPosition().y, -30.0f);

					for (int i = 0; i < EnemyList.size(); i++) {
						scene->RemoveEntity(EnemyList[i].getEnemy());
						EnemyList.erase(EnemyList.begin() + i);
						EnemyObjList.erase(EnemyObjList.begin() + i);
						i--;
					}

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy.get<Transform>().SetLocalPosition(15.0f, 0.0f, 0.0f);
						enemy.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E1 = Enemy(enemy);
					EnemyList.push_back(E1);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy2 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy2.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy2.get<Transform>().SetLocalPosition(6.0f, 2.9f, 0.0f);
						enemy2.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E2 = Enemy(enemy2);
					EnemyList.push_back(E2);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy3 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy3.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy3.get<Transform>().SetLocalPosition(30.0f, 0.0f, 0.0f);
						enemy3.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E3 = Enemy(enemy3);
					EnemyList.push_back(E3);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy4 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy4.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy4.get<Transform>().SetLocalPosition(21.0f, 4.0f, -1.2f);
						enemy4.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E4 = Enemy(enemy4);
					EnemyList.push_back(E4);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy5 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy5.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy5.get<Transform>().SetLocalPosition(40.0f, 0.0f, 0.0f);
						enemy5.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E5 = Enemy(enemy5);
					EnemyList.push_back(E5);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy6 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy6.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy6.get<Transform>().SetLocalPosition(44.0f, 0.0f, 0.0f);
						enemy6.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E6 = Enemy(enemy6);
					EnemyList.push_back(E6);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy7 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy7.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy7.get<Transform>().SetLocalPosition(50.0f, 0.0f, 0.0f);
						enemy7.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E7 = Enemy(enemy7);
					EnemyList.push_back(E7);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy8 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy8.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy8.get<Transform>().SetLocalPosition(57.0f, 0.0f, 0.0f);
						enemy8.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E8 = Enemy(enemy8);
					EnemyList.push_back(E8);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy9 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy9.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy9.get<Transform>().SetLocalPosition(67.4f, 0.0f, 0.0f);
						enemy9.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E9 = Enemy(enemy9);
					EnemyList.push_back(E9);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy10 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy10.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy10.get<Transform>().SetLocalPosition(73.7f, 0.0f, 0.0f);
						enemy10.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E10 = Enemy(enemy10);
					EnemyList.push_back(E10);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy11 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy11.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy11.get<Transform>().SetLocalPosition(93.9f, 3.0f, 0.0f);
						enemy11.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E11 = Enemy(enemy11);
					EnemyList.push_back(E11);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy12 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy12.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy12.get<Transform>().SetLocalPosition(101.5f, 4.6f, 0.0f);
						enemy12.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E12 = Enemy(enemy12);
					EnemyList.push_back(E12);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy13 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy13.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy13.get<Transform>().SetLocalPosition(113.96f, 4.6f, 0.0f);
						enemy13.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E13 = Enemy(enemy13);
					EnemyList.push_back(E13);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy14 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy14.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy14.get<Transform>().SetLocalPosition(125.0f, 0.0f, 0.0f);
						enemy14.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E14 = Enemy(enemy14);
					EnemyList.push_back(E14);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy15 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy15.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy15.get<Transform>().SetLocalPosition(140.0f, 0.0f, 0.0f);
						enemy15.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E15 = Enemy(enemy15);
					EnemyList.push_back(E15);

					EnemyObjList.push_back(scene->CreateEntity("enemy"));
					GameObject& enemy16 = EnemyObjList[EnemyObjList.size() - 1];
					{
						enemy16.emplace<RendererComponent>().SetMesh(knifevao).SetMaterial(knifeMat);
						enemy16.get<Transform>().SetLocalPosition(154.63f, 0.0f, 0.0f);
						enemy16.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
					}
					Enemy E16 = Enemy(enemy16);
					EnemyList.push_back(E16);



				}

			}
			asTimer = time.CurrentFrame;
			for (int i = 0; i < EnemyList.size(); i++) {

				EnemyList[i].AIPatrol();

				if (EnemyList[i].isDeath()) {
					scene->RemoveEntity(EnemyList[i].getEnemy());
					EnemyList.erase(EnemyList.begin() + i);
					EnemyObjList.erase(EnemyObjList.begin() + i);
					i--;
				}
			}
			cout<< time.CurrentFrame-asTimer <<endl;

			for (int x = 0; x < BulletList.size(); x++) {
				for (int y = 0; y < EnemyList.size(); y++) {
					if (HitCheck::AABB(BulletList[x].getHitBox(), EnemyList[y].getHitBox())) {
						BulletList[x].SetIsDeath(true);
						EnemyList[y].SetIsDeath(true); 
						SE_Die.Play();
					}
				} 
			}

			for (int y = 0; y < EnemyList.size(); y++) {
				if (HitCheck::AABB(P1.getHitBox(), EnemyList[y].getHitBox()) && P1.isDeath() == false) {
					P1.SetIsDeath(true);
					activeEffect = 4;
					SE_MelonDie.Play();
				} 
			}  
			//trap
			if (107.79 < P1.getPosition().x && 115.56 > P1.getPosition().x && P1.getPosition().y < 0.2 && P1.isDeath() == false) {
				P1.SetIsDeath(true);
				activeEffect = 4;
				SE_MelonDie.Play();
			}
			if (134 < P1.getPosition().x && 136.6 > P1.getPosition().x && P1.getPosition().y < 0.2 && P1.isDeath() == false) {
				P1.SetIsDeath(true);
				activeEffect = 4;
				SE_MelonDie.Play();
			}
			//Goal
			if (P1.getPosition().x >159 && P1.getPosition().y < -1 && !P1.isDeath()) {
				P1.SetIsDeath(true);

			}

			CamMoveX = 0;
			CamMoveY = 0;
			   
			activeEffect = 0;
			//////////////////////////////Gmae logic lololol////////////////////////////////////////
			if (glfwGetKey(BackendHandler::window, GLFW_KEY_J) == GLFW_PRESS && (Btime.CurrentFrame - BlastfpsTime) >= 1.0 / 3.0) {
				SE_shoot.Play();
				CamMoveX = (-20 + (rand() % 50)) * 0.002;
				CamMoveY = (-20 + (rand() % 50)) * 0.002;
				BulletObjList.push_back(scene->CreateEntity("bullet"));
				activeEffect = 4;

				GameObject& bullet = BulletObjList[BulletObjList.size() - 1];
				{
					bullet.emplace<RendererComponent>().SetMesh(seedsvao).SetMaterial(seedsMat);
					bullet.get<Transform>().SetLocalPosition(P1.getPosition().x, P1.getPosition().y+0.22, P1.getPosition().z);
					bullet.get<Transform>().SetLocalScale(0.7,0.7,0.7);
					bullet.get<Transform>().SetLocalRotation(0.0f, 0.0f, 0.0f);
				}
				Bullet B1 = Bullet(bullet, P1.returnFace());
				BulletList.push_back(B1);
				BlastfpsTime = Btime.CurrentFrame;
			}


			if ((time.CurrentFrame - lastfpsTime) >= 1.0 / 90.0) {//HZ limit
				P1.PlayerPhy(BackendHandler::window, time.DeltaTime, SE_Jump);

				cameraObject.get<Transform>().SetLocalPosition(glm::vec3(P1.getPosition().x, P1.getPosition().y, 0.0f) + glm::vec3(0 + CamMoveX, 2.5 + CamMoveY, 6));
				lastfpsTime = time.CurrentFrame;
			}
		}
		else if(sceneNumber==0) {
			if (glfwGetKey(BackendHandler::window, GLFW_KEY_D) == GLFW_PRESS && (Btime.CurrentFrame - BlastfpsTime) >= 1.0 / 3.0) {
				option = option - 1;
				if (option == -1) {
					option = 2;
				}
				BlastfpsTime = Btime.CurrentFrame;
			}
			else if (glfwGetKey(BackendHandler::window, GLFW_KEY_A) == GLFW_PRESS && (Btime.CurrentFrame - BlastfpsTime) >= 1.0 / 3.0) {
				option = option +1;
				if (option == 3) {
					option = 0;
				}
				BlastfpsTime = Btime.CurrentFrame;
			}
			if (option == 0) {

				seedsT.get<Transform>().SetLocalPosition(seedsT.get<Transform>().GetLocalPosition().x, seedsT.get<Transform>().GetLocalPosition().y, 3.0f);
				seedsT2.get<Transform>().SetLocalPosition(seedsT2.get<Transform>().GetLocalPosition().x, seedsT2.get<Transform>().GetLocalPosition().y, 2.5f);
				seedsT3.get<Transform>().SetLocalPosition(seedsT3.get<Transform>().GetLocalPosition().x, seedsT3.get<Transform>().GetLocalPosition().y, 2.5f);
				if (glfwGetKey(BackendHandler::window, GLFW_KEY_ENTER) == GLFW_PRESS) {
					BGM.SetParameter("Loop 2", 1); 
					BGM.SetParameter("Loop 3", 0);
					sceneNumber = 1;
					title.get<Transform>().SetLocalPosition(title.get<Transform>().GetLocalPosition().x, title.get<Transform>().GetLocalPosition().y, -30.0f);
					seedsT.get<Transform>().SetLocalPosition(seedsT.get<Transform>().GetLocalPosition().x, seedsT.get<Transform>().GetLocalPosition().y, -30.0f);
					seedsT2.get<Transform>().SetLocalPosition(seedsT2.get<Transform>().GetLocalPosition().x, seedsT2.get<Transform>().GetLocalPosition().y, -30.0f);
					seedsT3.get<Transform>().SetLocalPosition(seedsT3.get<Transform>().GetLocalPosition().x, seedsT3.get<Transform>().GetLocalPosition().y, -30.0f);
				}
			}else if (option == 1) {
				seedsT.get<Transform>().SetLocalPosition(seedsT.get<Transform>().GetLocalPosition().x, seedsT.get<Transform>().GetLocalPosition().y, 2.5f);
				seedsT2.get<Transform>().SetLocalPosition(seedsT2.get<Transform>().GetLocalPosition().x, seedsT2.get<Transform>().GetLocalPosition().y, 3.0f);
				seedsT3.get<Transform>().SetLocalPosition(seedsT3.get<Transform>().GetLocalPosition().x, seedsT3.get<Transform>().GetLocalPosition().y, 2.5f);

			}
			else if (option == 2) {
				seedsT.get<Transform>().SetLocalPosition(seedsT.get<Transform>().GetLocalPosition().x, seedsT.get<Transform>().GetLocalPosition().y, 2.5f);
				seedsT2.get<Transform>().SetLocalPosition(seedsT2.get<Transform>().GetLocalPosition().x, seedsT2.get<Transform>().GetLocalPosition().y, 2.5f);
				seedsT3.get<Transform>().SetLocalPosition(seedsT3.get<Transform>().GetLocalPosition().x, seedsT3.get<Transform>().GetLocalPosition().y, 3.0f);

			}
		}


		///////////////////////////////////////////////////////////////////////////////////////////	



	// Iterate over all the behaviour binding components
		scene->Registry().view<BehaviourBinding>().each([&](entt::entity entity, BehaviourBinding& binding) {
			// Iterate over all the behaviour scripts attached to the entity, and update them in sequence (if enabled)
			for (const auto& behaviour : binding.Behaviours) {
				if (behaviour->Enabled) {
					behaviour->Update(entt::handle(scene->Registry(), entity));
				}
			}
			});

		// Clear the screen
		basicEffect->Clear();
		for (int i = 0; i < effects.size(); i++)
		{
			effects[i]->Clear();
		}
		shadowBuffer->Clear();
		gBuffer->Clear();
		illuminationBuffer->Clear();

		glClearColor(1.0f, 1.0f, 1.0f, 0.3f);
		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Update all world matrices for this frame
		scene->Registry().view<Transform>().each([](entt::entity entity, Transform& t) {
			t.UpdateWorldMatrix();
			});

		// Grab out camera info from the camera object
		Transform& camTransform = cameraObject.get<Transform>();
		glm::mat4 view = glm::inverse(camTransform.LocalTransform());
		glm::mat4 projection = cameraObject.get<Camera>().GetProjection();
		glm::mat4 viewProjection = projection * view;

		//Set up light space martix
		glm::mat4 lightProjectionMartix = glm::ortho(-200.0f, 20.0f, -20.0f, 20.0f, -30.0f, 30.0f);
		glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(-illuminationBuffer->GetSunRef()._lightDirection), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 lightSpaceViewProj = lightProjectionMartix * lightViewMatrix;
		 
		//Set shadow stuff
		illuminationBuffer->SetLightSpaceViewProj(lightSpaceViewProj);
		glm::vec3 camPos = glm::inverse(view) * glm::vec4(0, 0, 0, 1);
		illuminationBuffer->SetCamPos(camPos);

		// Sort the renderers by shader and material, we will go for a minimizing context switches approach here,
		// but you could for instance sort front to back to optimize for fill rate if you have intensive fragment shaders
		renderGroup.sort<RendererComponent>([](const RendererComponent& l, const RendererComponent& r) {
			// Sort by render layer first, higher numbers get drawn last
			if (l.Material->RenderLayer < r.Material->RenderLayer) return true;
			if (l.Material->RenderLayer > r.Material->RenderLayer) return false;

			// Sort by shader pointer next (so materials using the same shader run sequentially where possible)
			if (l.Material->Shader < r.Material->Shader) return true;
			if (l.Material->Shader > r.Material->Shader) return false;

			// Sort by material pointer last (so we can minimize switching between materials)
			if (l.Material < r.Material) return true;
			if (l.Material > r.Material) return false;

			return false;
			});
		// Start by assuming no shader or material is applied
		Shader::sptr current = nullptr;
		ShaderMaterial::sptr currentMat = nullptr;

		glViewport(0, 0, shadowWitch, shadowHeight);
		shadowBuffer->Bind();

		renderGroup.each([&](entt::entity e, RendererComponent& renderer, Transform& transform) {
			// Render the mesh
			if (renderer.CastShadows)
			{
				BackendHandler::RenderVAO(simpleDepthShader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);
			}
			});

		//main_mesh.Draw(AnimShader, viewProjection, melonObj.get<Transform>().WorldTransform());
		
		if (P1.getIsPlayerMoving()) {
			main_mesh.Draw(AnimShader, viewProjection, glm::translate(melonObj.get<Transform>().LocalTransform(), glm::vec3(0, 2.0, 0)));
		}
		else {
			main_mesh2.Draw(AnimShader, viewProjection, glm::translate(melonObj.get<Transform>().LocalTransform(), glm::vec3(0, 2.0, 0)));
		}


		shadowBuffer->Unbind();

		glfwGetWindowSize(BackendHandler::window, &width, &height);

		glViewport(0, 0, width, height); 
		 
		gBuffer->Bind();

		// Iterate over the render group components and draw them
		renderGroup.each([&](entt::entity e, RendererComponent& renderer, Transform& transform) {
			// If the shader has changed, set up it's uniforms
			if (current != renderer.Material->Shader) {   
				current = renderer.Material->Shader;
				current->Bind();
				BackendHandler::SetupShaderForFrame(current, view, projection);   
			}
			// If the material has changed, apply it   
			if (currentMat != renderer.Material) {
				currentMat = renderer.Material;  
				currentMat->Apply();
			}


			// Render the mesh
			BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);
			});

		AnimMelonMat->Shader->Bind();
		AnimMelonMat->Apply(); 
		melonObj.get<Transform>().SetLocalScale(0.2, 0.2, 0.2);

		if (P1.getIsPlayerMoving()) {
			main_mesh.UpdateAnimation(main_mesh.GetAnimation(0), time.DeltaTime);
			main_mesh.Draw(AnimShader, viewProjection, glm::translate(melonObj.get<Transform>().LocalTransform(), glm::vec3(0, 2.0, 0)));
		}
		else {
			main_mesh2.UpdateAnimation(main_mesh2.GetAnimation(0), time.DeltaTime);
			main_mesh2.Draw(AnimShader, viewProjection, glm::translate(melonObj.get<Transform>().LocalTransform(), glm::vec3(0, 2.0, 0)));
		}


		//for (int i = 0; i < EnemyObjList.size(); i++) {
		//	AnimKnifeMat->Shader->Bind();
		//	AnimKnifeMat->Apply();
		//	EnemyObjList[i].get<Transform>().SetLocalScale(0.2, 0.2, 0.2);
		//	knife_mesh.UpdateAnimation(knife_mesh.GetAnimation(0), time.DeltaTime);
		//	knife_mesh.Draw(AnimShader, viewProjection, glm::translate(EnemyObjList[i].get<Transform>().LocalTransform(), glm::vec3(0, 2.0, 0)));
		//}






		gBuffer->Unbind();

		illuminationBuffer->BindBuffer(0); 

		skybox->Bind();
		BackendHandler::SetupShaderForFrame(skybox, view, projection);
		skyboxMat->Apply();
		BackendHandler::RenderVAO(skybox, meshVao, viewProjection, skyboxObj.get<Transform>(), lightSpaceViewProj);
		skybox->UnBind();

		illuminationBuffer->UnbindBuffer();

		shadowBuffer->BindDepthAsTexture(30);

		illuminationBuffer->ApplyEffect(gBuffer);

		shadowBuffer->UnbindTexture(30);

		if (drawGBuffer) {
			gBuffer->DrawBuffersToScreen();
		}
		else if (drawIllumBuffer) {
			illuminationBuffer->DrawIllumBuffer();
		}
		else {
			effects[activeEffect]->ApplyEffect(illuminationBuffer);
			effects[activeEffect]->DrawToScreen();
		}
		filmEffect->SetTime(time.CurrentFrame);

		// Draw our ImGui content
		BackendHandler::RenderImGui();

		scene->Poll();
		glfwSwapBuffers(BackendHandler::window);
		time.LastFrame = time.CurrentFrame;

	}
	engine.Shutdown();
	//directionalLightBuffer.Unbind(0);

	// Nullify scene so that we can release references
	Application::Instance().ActiveScene = nullptr;
	//Clean up the environment generator so we can release references
	EnvironmentGenerator::CleanUpPointers();
	BackendHandler::ShutdownImGui();


	Logger::Uninitialize();
	return 0;
}