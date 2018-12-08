#include "stdafx.h"
#include "PMScene.h"
#include "PMUtilities.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_LOADER_IMPLEMENTATION
#include "tiny_gltf_loader.h"
#include "JSONParsingHelpers.h"

namespace DXRPhotonMapper
{
    //------------------------------------------------------
    // Constructor
    //------------------------------------------------------
    PMScene::PMScene(UINT width, UINT height) : m_screenSize{ width, height }
    {
    }

    //------------------------------------------------------
    // Destructor
    //------------------------------------------------------
    PMScene::~PMScene()
    {
    }

    //------------------------------------------------------
    // PrintGLTFScene
    //------------------------------------------------------
    void PrintGLTFScene(tinygltf::Scene& m_gltfScene)
    {
        std::map<std::string, std::vector<std::string> >::const_iterator it(m_gltfScene.scenes.begin());
        std::map<std::string, std::vector<std::string> >::const_iterator itEnd(m_gltfScene.scenes.end());

        for (; it != itEnd; it++)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                //OutputDebugString(Lit->second[i] << ((i != (it->second.size() - 1)) ? ", " : "");
            }
            // OutputDebugString(L" ] ");
        }
    }

    PrimitiveType StringToPrimitiveType(const std::string& primTypeString)
    {
        if (primTypeString.compare("SquarePlane") == 0)
        {
            return PrimitiveType::SquarePlane;
        }
        else if (primTypeString.compare("Cube") == 0)
        {
            return PrimitiveType::Cube;
        }
        else
        {
            return PrimitiveType::Error;
        }
    }

    LightType StringToLightType(const std::string& lightTypeString)
    {
        if (lightTypeString.compare("PointLight") == 0)
        {
            return LightType::PointLight;
        }
        else if (lightTypeString.compare("AreaLight") == 0)
        {
            return LightType::AreaLight;
        }
        else if (lightTypeString.compare("SpotLight") == 0)
        {
            return LightType::SpotLight;
        }
        else
        {
            return LightType::Error;
        }
    }

    int StringToMaterialIndex(const std::string& materialName, const std::vector<Material>& m_materials)
    {
        for (size_t i = 0; i < m_materials.size(); ++i)
        {
            if (m_materials[i].m_name.compare(materialName) == 0)
            {
                return int(i);
            }
        }
        return -1;
    }


    bool LoadJSONCamera(picojson::value& root, Camera& camera)
    {
        const picojson::object& cameraObj = root.get("camera").get<picojson::object>();

        std::string err = "";

        double width = 0.0;
        if (!ParseNumberProperty(&width, &err, cameraObj, "width", true))
        {
            OutputDebugString(L"Error Parsing width");
        }

        double height = 0.0;
        if (!ParseNumberProperty(&height, &err, cameraObj, "height", true))
        {
            OutputDebugString(L"Error Parsing height");
        }

        double fov = 0.0;
        if (!ParseNumberProperty(&fov, &err, cameraObj, "fov", true))
        {
            OutputDebugString(L"Error Parsing fov");
        }

        std::vector<double> eye(3);
        if (!ParseNumberArrayProperty(&eye, &err, cameraObj, "eye", true))
        {
            OutputDebugString(L"Error Parsing eye");
        }

        std::vector<double> ref(3);
        if (!ParseNumberArrayProperty(&ref, &err, cameraObj, "ref", true))
        {
            OutputDebugString(L"Error Parsing ref");
        }

        std::vector<double> worldUp(3);
        if (!ParseNumberArrayProperty(&worldUp, &err, cameraObj, "worldUp", true))
        {
            OutputDebugString(L"Error Parsing worldUp");
        }

        camera.m_width = UINT(width);
        camera.m_height = UINT(height);
        camera.m_fov = float(fov);
        camera.m_eye = { float(eye[0]), float(eye[1]), float(eye[2]) };
        camera.m_ref = { float(ref[0]), float(ref[1]), float(ref[2]) };
        camera.m_up = { float(worldUp[0]), float(worldUp[1]), float(worldUp[2]) };

        // Just for Debugging
        std::wstringstream wstr;
        wstr << "Loading Camera " << std::endl;
        wstr << " m_width " << camera.m_width << std::endl;
        wstr << " m_height " << camera.m_height << std::endl;
        wstr << " m_fov " << camera.m_fov << std::endl;
        wstr << " m_eye " << camera.m_eye.x << ", " << camera.m_eye.y << ", " << camera.m_eye.z << std::endl;
        wstr << " m_ref " << camera.m_ref.x << ", " << camera.m_ref.y << ", " << camera.m_ref.z << std::endl;
        wstr << " m_up " << camera.m_up.x << ", " << camera.m_up.y << ", " << camera.m_up.z << std::endl;
        OutputDebugStringW(wstr.str().c_str());

        return true;
    }

    bool LoadJSONPrimitives(picojson::value& root, std::vector<Primitive>& primitives, const std::vector<Material>& m_materials)
    {
        std::string err = "";
        const picojson::array& primitiveArray = root.get("primitives").get<picojson::array>();

        primitives.clear();
        for (size_t i = 0; i < primitiveArray.size(); i++)
        {
            Primitive primitive = {};
            const picojson::object& primitiveObj = primitiveArray[i].get<picojson::object>();

            // Shape Type
            std::string shapeName;
            if (!ParseStringProperty(&shapeName, &err, primitiveObj, "shape", true))
            {
                OutputDebugString(L"Error Parsing shape");
            }
            primitive.m_primitiveType = StringToPrimitiveType(shapeName);

            // Primitive Identifier
            std::string primName;
            if (!ParseStringProperty(&primName, &err, primitiveObj, "name", true))
            {
                OutputDebugString(L"Error Parsing name");
            }
            primitive.m_name = primName;

            // Primitive Material ID
            std::string matName;
            if (!ParseStringProperty(&matName, &err, primitiveObj, "material", true))
            {
                OutputDebugString(L"Error Parsing material");
            }
            primitive.m_materialID = StringToMaterialIndex(matName, m_materials);

            // Primitive Transform
            picojson::object::const_iterator transformIt = primitiveObj.find("transform");
            if ((transformIt != primitiveObj.end()) && (transformIt->second).is<picojson::object>())
            {
                const picojson::object &transformObj = (transformIt->second).get<picojson::object>();

                std::vector<double> translate(3);
                if (!ParseNumberArrayProperty(&translate, &err, transformObj, "translate", false))
                {
                    OutputDebugString(L"Error Parsing translate");
                }

                std::vector<double> rotate(3);
                if (!ParseNumberArrayProperty(&rotate, &err, transformObj, "rotate", false))
                {
                    OutputDebugString(L"Error Parsing rotate");
                }

                std::vector<double> scale(3);
                if (!ParseNumberArrayProperty(&scale, &err, transformObj, "scale", false))
                {
                    OutputDebugString(L"Error Parsing scale");
                }

                primitive.m_translate = { float(translate[0]), float(translate[1]), float(translate[2]) };
                primitive.m_rotate = { float(rotate[0]), float(rotate[1]), float(rotate[2]) };
                primitive.m_scale = { float(scale[0]), float(scale[1]), float(scale[2]) };
            }
            primitives.push_back(primitive);
        }

        // For Debug Purpose only
        for (size_t i = 0; i < primitives.size(); ++i)
        {
            std::wstringstream wstr;
            wstr << "Found Primitive " << i << std::endl;
            wstr << " Name " << primitives[i].m_name.c_str() << std::endl;
            wstr << " Material " << primitives[i].m_materialID << std::endl;
            wstr << " translate " << primitives[i].m_translate.x << ", " << primitives[i].m_translate.y << ", " << primitives[i].m_translate.z << std::endl;
            wstr << " rotate " << primitives[i].m_rotate.x << ", " << primitives[i].m_rotate.y << ", " << primitives[i].m_rotate.z << std::endl;
            wstr << " scale " << primitives[i].m_scale.x << ", " << primitives[i].m_scale.y << ", " << primitives[i].m_scale.z << std::endl;
            OutputDebugStringW(wstr.str().c_str());
        }
        return true;
    }

    bool LoadJSONMaterials(picojson::value& root, std::vector<Material>& materials)
    {
        std::string err = "";
        const picojson::array& materialArray = root.get("materials").get<picojson::array>();

        materials.clear();
        for (size_t i = 0; i < materialArray.size(); i++)
        {
            Material material = {};
            const picojson::object& materialObj = materialArray[i].get<picojson::object>();

            // Material Name
            std::string matName;
            if (!ParseStringProperty(&matName, &err, materialObj, "name", true))
            {
                OutputDebugString(L"Error Parsing name");
            }
            material.m_name = matName;

            // Material albedo
            std::vector<double> albedo(3);
            if (!ParseNumberArrayProperty(&albedo, &err, materialObj, "albedo", false))
            {
                OutputDebugString(L"Error Parsing albedo");
            }

            // Material Type
            std::string matType;
            if (!ParseStringProperty(&matType, &err, materialObj, "type", true))
            {
                OutputDebugString(L"Error Parsing type");
            }

            if (matType.compare("MatteMaterial") == 0)
            {
                material.m_materialFlags = MaterialFlag::BSDF_DIFFUSE;

                DiffuseMat diffuseMat = {};
                diffuseMat.m_albedo = { float(albedo[0]), float(albedo[1]), float(albedo[2]) };
                diffuseMat.m_matIdentifier = MaterialFlag::BSDF_DIFFUSE;

                material.m_materials.push_back(diffuseMat);

                materials.push_back(material);
            }
            else
            {
                OutputDebugString(L"Unrecognized Material");
            }
        }

        // For Debug Purpose only
        for (size_t i = 0; i < materials.size(); ++i)
        {
            std::wstringstream wstr;
            wstr << "Found Material " << i << std::endl;
            wstr << " Name " << materials[i].m_name.c_str() << std::endl;
            OutputDebugStringW(wstr.str().c_str());
        }
        return true;

    }

    bool LoadJSONLights(picojson::value& root, std::vector<Light>& lights)
    {
        std::string err = "";
        const picojson::array& lightArray = root.get("lights").get<picojson::array>();

        lights.clear();
        for (size_t i = 0; i < lightArray.size(); i++)
        {
            Light light = {};
            const picojson::object& lightObj = lightArray[i].get<picojson::object>();

            // Light Name
            std::string lightName;
            if (!ParseStringProperty(&lightName, &err, lightObj, "name", true))
            {
                OutputDebugString(L"Error Parsing name");
            }
            light.m_name = lightName;

            // Light Intensity
            double intensity = 0.0;
            if (!ParseNumberProperty(&intensity, &err, lightObj, "intensity", true))
            {
                OutputDebugString(L"Error Parsing fov");
            }
            light.m_lightIntensity = float(intensity);

            // Light color
            std::vector<double> color(3);
            if (!ParseNumberArrayProperty(&color, &err, lightObj, "lightColor", true))
            {
                OutputDebugString(L"Error Parsing color");
            }
            light.m_lightColor = { float(color[0]), float(color[1]), float(color[2]) };

            // Light Transform
            picojson::object::const_iterator transformIt = lightObj.find("transform");
            if ((transformIt != lightObj.end()) && (transformIt->second).is<picojson::object>())
            {
                const picojson::object &transformObj = (transformIt->second).get<picojson::object>();

                std::vector<double> translate(3);
                if (!ParseNumberArrayProperty(&translate, &err, transformObj, "translate", false))
                {
                    OutputDebugString(L"Error Parsing translate");
                }

                std::vector<double> rotate(3);
                if (!ParseNumberArrayProperty(&rotate, &err, transformObj, "rotate", false))
                {
                    OutputDebugString(L"Error Parsing rotate");
                }

                std::vector<double> scale(3);
                if (!ParseNumberArrayProperty(&scale, &err, transformObj, "scale", false))
                {
                    OutputDebugString(L"Error Parsing scale");
                }

                light.m_translate = { float(translate[0]), float(translate[1]), float(translate[2]) };
                light.m_rotate = { float(rotate[0]), float(rotate[1]), float(rotate[2]) };
                light.m_scale = { float(scale[0]), float(scale[1]), float(scale[2]) };
            }

            // Light Type
            std::string lightType;
            if (!ParseStringProperty(&lightType, &err, lightObj, "type", true))
            {
                OutputDebugString(L"Error Parsing type");
            }
            light.m_lightType = StringToLightType(lightType);


            switch (light.m_lightType)
            {
            case LightType::PointLight:
            {
                // Drop off
                double dropOff = 0.0;
                if (!ParseNumberProperty(&dropOff, &err, lightObj, "dropOff", true))
                {
                    OutputDebugString(L"Error Parsing dropOff");
                }
                PointLight pointLight = {};
                pointLight.dropOff = float(dropOff);

                light.LightDesc.pointLight = pointLight;
            }
            break;
            case LightType::AreaLight:
            {
                // Two Sided
                bool twoSided = false;
                if (!ParseBooleanProperty(&twoSided, &err, lightObj, "twoSided", true))
                {
                    OutputDebugString(L"Error Parsing twoSided");
                }
                AreaLight areaLight = {};
                areaLight.isTwoSided = twoSided;

                light.LightDesc.areaLight = areaLight;
            }
            break;
            case LightType::SpotLight:
            {
                // Drop off
                double dropOff = 0.0;
                if (!ParseNumberProperty(&dropOff, &err, lightObj, "dropOff", true))
                {
                    OutputDebugString(L"Error Parsing dropOff");
                }

                // Cone Angle
                double coneAngle = 0.0;
                if (!ParseNumberProperty(&coneAngle, &err, lightObj, "coneAngle", true))
                {
                    OutputDebugString(L"Error Parsing coneAngle");
                }
                SpotLight spotLight = {};
                spotLight.dropOff = float(dropOff);
                spotLight.coneAngle = float(coneAngle);

                light.LightDesc.spotLight = spotLight;
            }
            break;
            case LightType::Error:
            default:
            {
                OutputDebugString(L"Unknown Light Type\n");
            }
            break;
            }

            lights.push_back(light);
        }

        // For Debug Purpose only
        for (size_t i = 0; i < lights.size(); ++i)
        {
            std::wstringstream wstr;
            wstr << "Found Light " << i << std::endl;
            wstr << " Name " << lights[i].m_name.c_str() << std::endl;
            wstr << " Intensity " << lights[i].m_lightIntensity << std::endl;
            wstr << " Color " << lights[i].m_lightColor.x << ", " << lights[i].m_lightColor.y << ", " << lights[i].m_lightColor.z << std::endl;
            wstr << " translate " << lights[i].m_translate.x << ", " << lights[i].m_translate.y << ", " << lights[i].m_translate.z << std::endl;
            wstr << " rotate " << lights[i].m_rotate.x << ", " << lights[i].m_rotate.y << ", " << lights[i].m_rotate.z << std::endl;
            wstr << " scale " << lights[i].m_scale.x << ", " << lights[i].m_scale.y << ", " << lights[i].m_scale.z << std::endl;
            OutputDebugStringW(wstr.str().c_str());
        }
        return true;
    }

    //------------------------------------------------------
    // LoadPicoScene
    //------------------------------------------------------
    bool PMScene::LoadPicoScene(const char *str, unsigned int length)
    {
        std::wstringstream wstr;
        picojson::value v;
        std::string perr = picojson::parse(v, str, str + length);

        if (!perr.empty())
        {
            OutputDebugString(L"Could not parse JSON file\n");
            return false;
        }

        if (!(v.contains("camera") && v.get("camera").is<picojson::object>()))
        {
            OutputDebugString(L"JSON File Does not contain camera\n");
            return false;
        }

        if (!(v.contains("primitives") && v.get("primitives").is<picojson::array>()))
        {
            OutputDebugString(L"JSON File Does not contain primitives\n");
            return false;
        }

        if (!(v.contains("lights") && v.get("lights").is<picojson::array>()))
        {
            OutputDebugString(L"JSON File Does not contain lights\n");
            return false;
        }

        if (!(v.contains("materials") && v.get("materials").is<picojson::array>()))
        {
            OutputDebugString(L"JSON File Does not contain materials\n");
            return false;
        }

        if (!LoadJSONCamera(v, m_camera))
        {
            OutputDebugString(L"Error Parsing Camera\n");
        }

        if (!LoadJSONMaterials(v, m_materials))
        {
            OutputDebugString(L"Error Parsing Materials\n");
        }

        if (!LoadJSONPrimitives(v, m_primitives, m_materials))
        {
            OutputDebugString(L"Error Parsing Primitives\n");
        }

        if (!LoadJSONLights(v, m_lights))
        {
            OutputDebugString(L"Error Parsing Lights\n");
        }

        /*

        // Test - just to check if we can read some values from JSON

        std::string sampleString = v.get("string").get<std::string>();
        double sampleNumber = v.get("number").get<double>();
        int sampleNumber2 = int(v.get("integer").get<double>());

        wstr << L"Sample String " << sampleString.c_str() << L"\n";
        wstr << L"Sample Float " << sampleNumber << L"\n";
        wstr << L"Sample Integer " << sampleNumber2 << L"\n";


        OutputDebugStringW(wstr.str().c_str());


        */

        return true;
    }

    //------------------------------------------------------
    // LoadGLTFBufferViews
    //------------------------------------------------------
    void LoadGLTFBufferViews(tinygltf::Scene& m_gltfScene, std::map<std::string, BufferHolder>& m_gltfBufferHolders)
    {
        std::map<std::string, tinygltf::BufferView>::const_iterator it(m_gltfScene.bufferViews.begin());
        std::map<std::string, tinygltf::BufferView>::const_iterator itEnd(m_gltfScene.bufferViews.end());

        for (; it != itEnd; it++)
        {
            const std::string key = it->first;
            const tinygltf::BufferView &bufferView = it->second;

            if (bufferView.target == 0)
            {
                continue; // Unsupported bufferView.
            }

            const tinygltf::Buffer &buffer = m_gltfScene.buffers.at(bufferView.buffer);

            BufferHolder bufferHolder = {};
            bufferHolder.m_bufferStart = &buffer.data.front() + bufferView.byteOffset;
            bufferHolder.m_length = bufferView.byteLength;

            m_gltfBufferHolders.insert(std::make_pair(key, bufferHolder));
        }
    }

    //------------------------------------------------------
    // LoadGLTFMeshes
    //------------------------------------------------------
    void LoadGLTFMeshes()
    {

    }


    //------------------------------------------------------
    // LoadJSONScene
    //------------------------------------------------------
    bool PMScene::LoadJSONScene(const std::string& fileName)
    {
        // 1.Read Data from file
        std::ifstream file(fileName.c_str());
        if (!file)
        {
            OutputDebugString(L"Failed to open file\n");
            return false;
        }

        file.seekg(0, file.end);
        const size_t sz = static_cast<size_t>(file.tellg());
        std::vector<char> buf(sz);

        if (sz == 0)
        {
            OutputDebugString(L"Empty JSON file\n");
            return false;
        }

        file.seekg(0, file.beg);
        file.read(&buf.at(0), static_cast<std::streamsize>(sz));
        file.close();

        return LoadPicoScene(&buf.at(0), static_cast<unsigned int>(buf.size()));
    }

    //------------------------------------------------------
    // LoadScene
    //------------------------------------------------------
    bool PMScene::LoadGLTFScene(const std::string& fileName)
    {
        tinygltf::Scene m_gltfScene;
        tinygltf::TinyGLTFLoader loader;
        std::string err;
        std::string ext = getFilePathExtension(fileName);

        // Load the GLTF file
        const bool ret = [&]
        {
            if (ext.compare("glb") == 0)
            {
                // assume binary glTF.
                return loader.LoadBinaryFromFile(&m_gltfScene, &err, fileName.c_str());
            }
            else
            {
                // assume ascii glTF.
                return loader.LoadASCIIFromFile(&m_gltfScene, &err, fileName.c_str());
            }
        }();

        if (!err.empty())
        {
        }

        if (!ret)
        {

            OutputDebugString(L"Failed to parse glTF\n");
            getchar();
            return false;
        }

        // Print the scene info to console
        PrintGLTFScene(m_gltfScene);

        // Read all raw buffers
        LoadGLTFBufferViews(m_gltfScene, m_gltfBufferHolders);

        LoadGLTFMeshes();

        return true;
    }


}
