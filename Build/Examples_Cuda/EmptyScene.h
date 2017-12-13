#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\SphereCollisionShape.h>

//Fully striped back scene to use as a template for new scenes.
class EmptyScene : public Scene
{
public:
	EmptyScene(const std::string& friendly_name) 
		: Scene(friendly_name)
	{
	}

	virtual ~EmptyScene()
	{
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();

		//Who doesn't love finding some common ground?
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.5f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	}

	void EmptyScene::OnCleanupScene()
	{
		//Just delete all created game objects 
		//  - this is the default command on any Scene instance so we don't really need to override this function here.
		Scene::OnCleanupScene();
	}

};