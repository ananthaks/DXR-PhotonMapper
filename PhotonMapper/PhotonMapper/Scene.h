#pragma once
#include "RaytracingHlslCompat.h"

struct Geometry
{
	std::vector<Vertex> m_vertices;
	std::vector<Index> m_indices;
};

struct Material
{
	DirectX::XMFLOAT3 m_baseColor;
};

struct Light
{
	DirectX::XMFLOAT3 m_lightPos;
	DirectX::XMFLOAT3 m_lightColor;
};

struct Camera
{
	DirectX::XMFLOAT3 m_cameraPos;
	DirectX::XMFLOAT3 m_eye;
	DirectX::XMFLOAT3 m_look;
	DirectX::XMFLOAT3 m_ref;
	float m_fov;
};

class Scene
{
private:
	std::vector<Geometry> m_sceneGeoms;
	std::vector<Material> m_sceneMaterials;

private:

	bool LoadGeometry(std::string objectId);
	bool LoadMaterial(std::string materialId);
	bool LoadCamera();

public:
    Scene();
    ~Scene();

	bool LoadScene(const std::string& fileName);

};

