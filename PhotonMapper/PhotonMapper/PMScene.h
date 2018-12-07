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

    enum class MaterialFlag
    {
        BSDF_REFLECTION = 1 << 0,   // This BxDF handles rays that are reflected off surfaces
        BSDF_TRANSMISSION = 1 << 1, // This BxDF handles rays that are transmitted through surfaces
        BSDF_DIFFUSE = 1 << 2,      // This BxDF represents diffuse energy scattering, which is uniformly random
        BSDF_GLOSSY = 1 << 3,       // This BxDF represents glossy energy scattering, which is biased toward certain directions
        BSDF_SPECULAR = 1 << 4,     // This BxDF handles specular energy scattering, which has no element of randomness
        BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION
    };

    struct BaseMat
    {
        DirectX::XMFLOAT3 m_albedo;
        MaterialFlag m_matIdentifier;
    };

    struct DiffuseMat : BaseMat
    {
    };

    struct ReflectiveMat : BaseMat
    {
        float roughness;
    };

    struct TransmissiveMat : BaseMat
    {
        float refractiveIndex;
    };

    struct SpecularMat : BaseMat
    {
        float sigma;
    };

    struct Material
    {
        std::string m_name;
        MaterialFlag m_materialFlags;
        std::vector<BaseMat> m_materials;
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

    enum class LightType
    {
        PointLight = 0,
        AreaLight,
        SpotLight,
        Error,
    };

    struct AreaLight
    {
        bool isTwoSided;
    };

    struct PointLight
    {
        float dropOff;
    };

    struct SpotLight
    {
        float coneAngle;
        float dropOff;
    };

    struct Light
    {
        std::string m_name;
        LightType m_lightType;
        float m_lightIntensity;
        DirectX::XMFLOAT3 m_lightColor;
        DirectX::XMFLOAT3 m_translate;
        DirectX::XMFLOAT3 m_rotate;
        DirectX::XMFLOAT3 m_scale;

        union
        {
            PointLight pointLight;
            AreaLight areaLight;
            SpotLight spotLight;
        } LightDesc;
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

