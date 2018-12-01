#pragma once

#include <array>
#include <map>

#include "RaytracingHlslCompat.h"

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_LOADER_IMPLEMENTATION
#include "tiny_gltf_loader.h"


namespace DXRPhotonMapper
{
    typedef unsigned char ByteBuffer; 

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

    typedef struct 
    {
        const ByteBuffer* m_bufferStart;
        std::size_t m_length;
    }BufferHolder;

    class PMScene
    {
    private:

        std::array<UINT, 2> m_screenSize;
	    std::vector<Geometry> m_sceneGeoms;
	    std::vector<Material> m_sceneMaterials;

        tinygltf::Scene m_gltfScene;
        std::map<std::string, BufferHolder> m_gltfBufferHolders;

    private:

        //------------------------------------------------------
        // PrintGLTFScene
        //------------------------------------------------------
        void PrintGLTFScene();

        //------------------------------------------------------
        // LoadGLTFBufferViews
        //------------------------------------------------------
	    void LoadGLTFBufferViews();

        //------------------------------------------------------
        // LoadGLTFMeshes
        //------------------------------------------------------
        void LoadGLTFMeshes();

    public:
        //------------------------------------------------------
        // Constructor
        //------------------------------------------------------
        PMScene(UINT width, UINT height);

        //------------------------------------------------------
        // Destructor
        //------------------------------------------------------
        ~PMScene();

        //------------------------------------------------------
        // LoadScene
        //------------------------------------------------------
	    bool LoadGLTFScene(const std::string& fileName);

    };

}

