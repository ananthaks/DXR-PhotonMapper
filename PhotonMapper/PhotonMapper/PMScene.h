#pragma once

#include <array>
#include <map>

#include "RaytracingHlslCompat.h"

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
        std::string m_name;
        DirectX::XMFLOAT3 m_baseColor;
    };

    enum class PrimitiveType
    {
        SquarePlane = 0,
        Cube,
        Error,
    };

    struct Primitive
    {
        std::string m_name;
        PrimitiveType m_primitiveType;
        int m_materialID;
        DirectX::XMFLOAT3 m_translate;
        DirectX::XMFLOAT3 m_rotate;
        DirectX::XMFLOAT3 m_scale;
    };

    struct Light
    {
        DirectX::XMFLOAT3 m_lightPos;
        DirectX::XMFLOAT3 m_lightColor;
    };

    struct Camera
    {
        DirectX::XMFLOAT3 m_eye;
        DirectX::XMFLOAT3 m_ref;
        DirectX::XMFLOAT3 m_up;
        float m_fov;
        UINT m_width;
        UINT m_height;
    };

    typedef struct 
    {
        const ByteBuffer* m_bufferStart;
        std::size_t m_length;
    }BufferHolder;


    class PMScene
    {
    private:

        Camera m_camera = {};
        std::array<UINT, 2> m_screenSize;


	    std::vector<Geometry> m_sceneGeoms;
	    std::vector<Primitive> m_primitives;
	    std::vector<Material> m_materials;
	    std::vector<Light> m_lights;

        std::map<std::string, BufferHolder> m_gltfBufferHolders;
        
    private:

        bool LoadPicoScene(const char *str, unsigned int length);



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
        // LoadGLTFScene
        //------------------------------------------------------
	    bool LoadGLTFScene(const std::string& fileName);

        //------------------------------------------------------
        // LoadJSONScene
        //------------------------------------------------------
	    bool LoadJSONScene(const std::string& fileName);
    };

}

