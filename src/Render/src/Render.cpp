#include "Render/Render.h"
#include "assimp/scene.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Logger/Logger.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <set>

void Canella::load_asset_mesh(ModelMesh& model, const ::std::string& assetsPath, const std::string& source)
{
    static const int assimpFlags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices;
    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(assetsPath + "\\" + source, aiProcessPreset_TargetRealtime_MaxQuality );
    if (!assimpScene || assimpScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
        Logger::Error(importer.GetErrorString());

    auto& [positions,normal,indices,meshes,matrix,meshlet_composition] = model;
    indices.clear();
    positions.clear();
    meshes.resize(assimpScene->mNumMeshes);
    uint32_t index_offset = 0;
    uint32_t vertex_offset = 0;
    for (unsigned int i = 0; i < model.meshes.size(); ++i)
    {
        const aiMesh* assimp_mesh = assimpScene->mMeshes[i];
        meshes[i].index_offset = index_offset;
        meshes[i].vertex_offset = vertex_offset;
        meshes[i].index_count = assimp_mesh->mNumFaces * 3;
        meshes[i].vertex_count = assimp_mesh->mNumVertices;

        for (unsigned int j = 0; j < assimp_mesh->mNumVertices; j++)
        {
            auto v3 = glm::make_vec4(&assimp_mesh->mVertices[j].x);
            auto c = assimp_mesh->mColors[0];
            Vertex vertex;
            vertex.vertex = v3;
            positions.push_back(vertex);
        }
        vertex_offset +=assimp_mesh->mNumVertices;
        for (size_t face = 0; face < assimp_mesh->mNumFaces; ++face)
            for (uint32_t face_index = 0; face_index < 3; ++face_index )
                indices.push_back(static_cast<uint32_t>(assimp_mesh->mFaces[face].mIndices[face_index]));

            index_offset +=assimp_mesh->mNumFaces *3;

        //Load Meshlets
        load_meshlet(meshlet_composition,model,i);
        }
}

void Canella::load_meshlet(Canella::Meshlet& canellaMeshlet, Canella::ModelMesh &model,int mesh_index) {

    constexpr size_t max_vertices = 64;
    constexpr size_t max_triangles = 128;
    auto vertx_offset = model.meshes[mesh_index].vertex_offset;
    auto vertex_count = model.meshes[mesh_index].vertex_count;
    auto index_offset = model.meshes[mesh_index].index_offset;
    auto index_count = model.meshes[mesh_index].index_count;

    std::vector<Vertex>   positions(model.positions.begin() + vertx_offset,model.positions.begin() + vertx_offset + vertex_count);
    std::vector<uint32_t> indices(model.indices.begin() + index_offset,model.indices.begin() + index_offset + index_count);

    const size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), max_vertices, max_triangles);
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<MeshletBound> bounds;
    meshlets.resize(max_meshlets);
    std::vector<unsigned int> meshlet_vertices(max_meshlets * max_vertices);
    std::vector<unsigned char> meshlet_triangles(max_meshlets * max_triangles * 3);
    const size_t meshlet_count = meshopt_buildMeshlets(meshlets.data(),
                                                           meshlet_vertices.data(),
                                                           meshlet_triangles.data(),
                                                           indices.data(),
                                                           indices.size(),
                                                           &positions[0].vertex.x,
                                                           positions.size(),
                                                           sizeof(Vertex),
                                                           max_vertices,
                                                           max_triangles,
                                                           .7f);

    const meshopt_Meshlet& last = meshlets[meshlet_count - 1];
    meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
    meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
    meshlets.resize(meshlet_count);

    for (const auto& m : meshlets)
    {
        auto bound = meshopt_Bounds(meshopt_computeMeshletBounds(&meshlet_vertices[m.vertex_offset],
                                                         &meshlet_triangles[m.triangle_offset],
                                                         m.triangle_count, &positions[0].vertex.x,
                                                         positions.size(),
                                                         sizeof(Vertex)));

        MeshletBound meshletBound {};
        meshletBound.cone_apex = glm::vec4(bound.cone_apex[0],bound.cone_apex[1],bound.cone_apex[2],0.f);
        meshletBound.cone_axis = glm::vec4(bound.cone_axis[0],bound.cone_axis[1],bound.cone_axis[2],0.f);
        meshletBound.cone_cutoff = glm::vec4(bound.cone_cutoff,0,0,0.0f);
        bounds.push_back(meshletBound);
    }

    model.meshes[mesh_index].meshlet_vertex_offset = canellaMeshlet.meshlet_vertices.size();
    model.meshes[mesh_index].meshlet_triangle_offset = canellaMeshlet.meshlet_triangles.size();
    model.meshes[mesh_index].meshlet_count = meshlet_count;
    model.meshes[mesh_index].meshlet_offset = canellaMeshlet.meshlets.size();

    canellaMeshlet.meshlets.insert(canellaMeshlet.meshlets.end(),meshlets.begin(),meshlets.end());
    canellaMeshlet.meshlet_triangles.insert(canellaMeshlet.meshlet_triangles.end(),meshlet_triangles.begin(),meshlet_triangles.end());
    canellaMeshlet.meshlet_vertices.insert(canellaMeshlet.meshlet_vertices.end(),meshlet_vertices.begin(),meshlet_vertices.end());
    canellaMeshlet.bounds.insert(canellaMeshlet.bounds.end(),bounds.begin(),bounds.end());
}
