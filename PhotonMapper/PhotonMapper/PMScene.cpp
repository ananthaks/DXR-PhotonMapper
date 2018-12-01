#include "stdafx.h"
#include "PMScene.h"
#include "tiny_gltf_loader.h"
#include "PMUtilities.h"
#include <iostream>



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
    // LoadScene
    //------------------------------------------------------
    bool PMScene::LoadGLTFScene(const std::string& fileName)
    {
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
            printf("Err: %s\n", err.c_str());
        }

        if (!ret) 
        {

            printf("Failed to parse glTF\n");
            getchar();
            return false;
        }

        // Print the scene info to console
        PrintGLTFScene();

        // Read all raw buffers
        LoadGLTFBufferViews();

        return true;
    }

    void PMScene::PrintGLTFScene()
    {
        std::map<std::string, std::vector<std::string> >::const_iterator it(m_gltfScene.scenes.begin());
        std::map<std::string, std::vector<std::string> >::const_iterator itEnd(m_gltfScene.scenes.end());

        for (; it != itEnd; it++) 
        {
            for (size_t i = 0; i < it->second.size(); i++) 
            {
                std::cout << it->second[i] << ((i != (it->second.size() - 1)) ? ", " : "");
            }
            std::cout << " ] " << std::endl;
        }
    }

    void PMScene::LoadGLTFBufferViews()
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

            m_bufferHolders.insert(std::make_pair(key, bufferHolder));
        }
    }
}
