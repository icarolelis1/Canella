#include "Render/Render.h"
#include "assimp/scene.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Logger/Logger.hpp>

Canella::Render::Render(nlohmann::json &configFile){};

static void Canella::LoadAssetMesh(Mesh& mesh,const::std::string& assetsPath, const std::string& source )
{
    static const int assimpFlags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices;
    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(assetsPath + "\\" + source,aiProcess_Triangulate);
    if (!assimpScene || assimpScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
        Logger::Error(importer.GetErrorString());
    
}
