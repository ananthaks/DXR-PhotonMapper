#include "stdafx.h"
#include "Scene.h"
#include "tiny_gltf_loader.h"
#include "PMUtilities.h"

Scene::Scene()
{
	
}
Scene::~Scene()
{
}

bool Scene::LoadScene(const std::string& fileName)
{
	
	return true;
}

bool Scene::LoadGeometry(std::string objectId)
{
	return false;
}

bool Scene::LoadMaterial(std::string materialId)
{
	return false;
}

bool Scene::LoadCamera()
{
	return false;
}