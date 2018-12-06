#include "stdafx.h"
#include "PMScene.h"
#include "PMUtilities.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_LOADER_IMPLEMENTATION
#include "tiny_gltf_loader.h"

namespace DXRPhotonMapper
{
    //------------------------------------------------------
    // Constructor
    //------------------------------------------------------
    PMScene::PMScene(UINT width, UINT height) : m_screenSize{width, height}
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

    

    //------------------------------------------------------
    // LoadPicoScene
    //------------------------------------------------------
    bool LoadPicoScene(const char *str, unsigned int length)
    {
        std::wstringstream wstr;
        picojson::value v;
        std::string perr = picojson::parse(v, str, str + length);

        if (!perr.empty()) 
        {
            OutputDebugString(L"Could not parse JSON file");
            return false;
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
