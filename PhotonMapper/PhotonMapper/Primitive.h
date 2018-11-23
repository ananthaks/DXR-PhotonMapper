#pragma once
#include <vector>

struct PrimVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 color;
    DirectX::XMFLOAT3 normal;
};

class Primitive
{
private:
    std::vector<PrimVertex> m_vertices;
    std::vector<uint16_t> m_indices;
    DirectX::XMFLOAT4X4 m_modelMatrix;

public:
    Primitive();

    ~Primitive();

    void CreateCube(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);

    void CreatePlane(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);

    void CreateMesh();

    const DirectX::XMFLOAT4X4& GetModelMatrix() const;

    const PrimVertex* GetVertices() const;

    const uint16_t* GetIndices() const;
};

