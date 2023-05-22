#include "Render/Render.h"
#include "assimp/scene.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Logger/Logger.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <set>
Canella::Render::Render(nlohmann::json& configFile)
{
}

/**
 * \brief 
 * \param mesh 
 * \param assetsPath 
 * \param source 
 */
void Canella::load_asset_mesh(ModelMesh& model, const ::std::string& assetsPath, const std::string& source)
{
    static const int assimpFlags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices;
    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(assetsPath + "\\" + source, aiProcessPreset_TargetRealtime_MaxQuality );
    if (!assimpScene || assimpScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
        Logger::Error(importer.GetErrorString());

    model.meshes.resize(assimpScene->mNumMeshes);

    for (unsigned int i = 0; i < model.meshes.size(); ++i)
    {
        const aiMesh* assimp_mesh = assimpScene->mMeshes[i];
        auto& [positions,
            normal,
            indices
        ] = model.meshes[i];

        indices.clear();
        positions.clear();
        for (unsigned int j = 0; j < assimp_mesh->mNumVertices; j++)
        {
            auto v3 = glm::make_vec4(&assimp_mesh->mVertices[j].x);
            Vertex vertex;
            vertex.vertex = v3;
            positions.push_back(vertex);
        }

        for (size_t face = 0; face < assimp_mesh->mNumFaces; ++face)
            for (uint32_t face_index = 0; face_index < 3; ++face_index ){

                if(assimp_mesh->mFaces[face].mIndices[face_index] >assimp_mesh->mNumVertices)
                    Canella::Logger::Error("INVALID INDICE %d",assimp_mesh->mFaces[face].mIndices[face_index]);
                indices.push_back(static_cast<uint32_t>(assimp_mesh->mFaces[face].mIndices[face_index]));
            }


        for(auto indice : indices){
            if(indices[i] >= positions.size())
                indices[i];
        }


    }


}

void Canella::load_meshlet(Canella::Meshlet& canellaMeshlet, const Canella::Mesh &mesh) {

    constexpr size_t max_vertices = 64;
    constexpr size_t max_triangles = 124;
    constexpr float cone_weight = 0.5f;

    auto &indices = mesh.indices;
    auto &positions = mesh.positions;
    const size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), max_vertices, max_triangles);
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<meshopt_Bounds> bounds;
    meshlets.resize(max_meshlets);
    std::vector<unsigned int> meshlet_vertices(max_meshlets * max_vertices);
    std::vector<unsigned char> meshlet_triangles(max_meshlets * max_triangles * 3);
    const size_t meshlet_count = meshopt_buildMeshletsScan(meshlets.data(), meshlet_vertices.data(),
                                                       meshlet_triangles.data(),
                                                       indices.data(),
                                                       indices.size(),positions.size(),max_vertices,max_triangles);

    const meshopt_Meshlet& last = meshlets[meshlet_count - 1];
    meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
    meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
    meshlets.resize(meshlet_count);

    for (const auto& m : meshlets)
    {
        bounds.emplace_back(meshopt_computeMeshletBounds(&meshlet_vertices[m.vertex_offset],
                                                         &meshlet_triangles[m.triangle_offset],
                                                         m.triangle_count, &positions[0].vertex.x,
                                                         positions.size(),
                                                         sizeof(Vertex)));
    }
    canellaMeshlet.meshlets = meshlets;
    canellaMeshlet.bounds = bounds;
}
