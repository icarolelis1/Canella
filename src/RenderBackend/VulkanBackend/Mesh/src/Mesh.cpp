#include "Mesh/Mesh.h"
#include "assimp/scene.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Logger/Logger.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <set>

using namespace Canella;

struct Vec4Hash {
    std::size_t operator()(const glm::vec4& v) const {
        std::size_t seed = 0;
        for (int i = 0; i < 4; ++i) {
            seed ^= std::hash<float>{}(v[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

void MeshProcessing::simplify_indices( std::vector<Vertex>& vertices, std::vector<uint32_t>& indices )
{
    std::unordered_map<glm::vec4, uint32_t, Vec4Hash> vertex_to_index;
    std::vector<uint32_t> simplified_indices;

    for (auto& index : indices)
    {
        const auto& vertex = vertices[index];
        const auto& position = vertex.position;

        auto it = vertex_to_index.find(position);
        if (it != vertex_to_index.end())
        {
            simplified_indices.push_back(it->second);
        }
        else
        {
            uint32_t newIndex = static_cast<uint32_t>(vertex_to_index.size());
            vertex_to_index[position] = newIndex;
            simplified_indices.push_back(newIndex);
        }
    }
    indices = std::move(simplified_indices);
}

MeshProcessing::Mesh::Mesh(std::vector<uint32_t>indices)
{
    for(int i = 0 ; i < indices.size(); i+= 3)
    {
        auto v1 = VertexHe{indices[i]};
        auto v2 = VertexHe{indices[i + 1]};
        auto v3 = VertexHe{indices[i + 2]};

        std::pair<uint32_t,uint32_t> edge1(v1.id,v2.id);
        std::pair<uint32_t,uint32_t> edge2(v2.id,v3.id);
        std::pair<uint32_t,uint32_t> edge3(v3.id,v1.id);

        auto face = Face();

        edges[edge1] = new HalfEdge();
        v1.he = edges[edge1];
        edges[edge1]->triangle = &face;
        edges[edge2] = new HalfEdge();
        v2.he = edges[edge2];
        edges[edge2]->triangle = &face;

        edges[edge3] = new HalfEdge();
        v3.he = edges[edge3];
        edges[edge3]->triangle = &face;
        face.he = edges[edge1];
        face.vertices[0] = v1;
        face.vertices[1] = v2;
        face.vertices[2] = v3;


        triangles.push_back(face);

        edges[edge1]->next = edges[edge2];
        edges[edge2]->next = edges[edge3];
        edges[edge3]->next = edges[edge1];

        if(edges.find(std::pair(v2.id,v1.id)) != edges.end())
        {
            edges[std::pair(v2.id,v1.id)]->twin = edges[std::pair(v1.id,v2.id)];
            edges[std::pair(v1.id,v2.id)]->twin = edges[std::pair(v2.id,v1.id)];
        }

        if(edges.find(std::pair(v3.id,v2.id)) != edges.end())
        {
            edges[std::pair(v2.id,v3.id)]->twin = edges[std::pair(v3.id,v2.id)];
            edges[std::pair(v3.id,v2.id)]->twin = edges[std::pair(v2.id,v3.id)];
        }

        if(edges.find(std::pair(v1.id,v3.id)) != edges.end())
        {
            edges[std::pair(v1.id,v3.id)]->twin = edges[std::pair(v3.id,v1.id)];
            edges[std::pair(v3.id,v1.id)]->twin = edges[std::pair(v1.id,v3.id)];
        }

        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
    }
}

MeshProcessing::Mesh::~Mesh()
{
    for(auto& edge : edges)
        delete edge.second;
}

void MeshProcessing::Mesh::classify_triangles() {

    auto n = 0;
    for(auto edge : edges)
    {
        if(edge.second->twin)
            n++;
    };
    Logger::Info("Number Of Triangles : %d",n);
}

int MeshProcessing::count_boundaries(MeshProcessing::Mesh& mesh)
{
    auto boundaries = 0;

    for (auto& he : mesh.edges)
    {
      if(he.second->twin == nullptr)
          boundaries++;
    }
    return boundaries;
}

void MeshProcessing::load_asset_mesh(ModelMesh &model, const ::std::string &assetsPath, const std::string &source)
{
    Assimp::Importer importer;
    const aiScene *assimpScene = importer.ReadFile(assetsPath + "\\" + source, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!assimpScene || assimpScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
        Logger::Error(importer.GetErrorString());

    auto &[positions, normal, indices, meshes, matrix, meshlet_composition] = model;
    indices.clear();
    positions.clear();
    meshes.resize(assimpScene->mNumMeshes);
    uint32_t index_offset = 0;
    uint32_t vertex_offset = 0;
    for (unsigned int i = 0; i < model.meshes.size(); ++i)
    {
        const aiMesh *assimp_mesh = assimpScene->mMeshes[i];
        meshes[i].index_offset    = index_offset;
        meshes[i].vertex_offset   = vertex_offset;
        meshes[i].index_count     = assimp_mesh->mNumFaces * 3;
        meshes[i].vertex_count    = assimp_mesh->mNumVertices;

        for (unsigned int j = 0; j < assimp_mesh->mNumVertices; j++)
        {
            auto v3         = glm::make_vec4(&assimp_mesh->mVertices[j].x);
            Vertex vertex;
            vertex.position = v3;
            positions.push_back(vertex);
        }

        for (size_t face = 0; face < assimp_mesh->mNumFaces; ++face)
            for (uint32_t face_index = 0; face_index < 3; ++face_index)
                indices.push_back(static_cast<uint32_t>(assimp_mesh->mFaces[face].mIndices[face_index]));

        index_offset = indices.size();
        vertex_offset = positions.size();

    /*    size_t index_count = indices.size();

        std::vector<uint32_t> remap(index_count);
        size_t vertex_count = meshopt_generateVertexRemap(remap.data(), 0, index_count, positions.data(), index_count, sizeof(Vertex));

        std::vector<Vertex> vertices(vertex_count);
        std::vector<uint32_t> new_indices(index_count);

        meshopt_remapVertexBuffer(vertices.data(), positions.data(), index_count, sizeof(Vertex), remap.data());
        meshopt_remapIndexBuffer(new_indices.data(), indices.data(), index_count, remap.data());

        meshopt_optimizeVertexCache(new_indices.data(), indices.data(), index_count, vertex_count);
        meshopt_optimizeVertexFetch(vertices.data(), indices.data(), index_count, vertices.data(), vertex_count, sizeof(Vertex));
*/
        MeshProcessing::build_meshlets(meshlet_composition, model, i);
    }
}

void MeshProcessing::build_meshlets(Canella::Meshlet &canellaMeshlet, Canella::ModelMesh &model, int mesh_index)
{
    constexpr size_t max_vertices = 64;
    constexpr size_t max_triangles = 128;
    auto vertx_offset = model.meshes[mesh_index].vertex_offset;
    auto vertex_count = model.meshes[mesh_index].vertex_count;
    auto index_offset = model.meshes[mesh_index].index_offset;
    auto index_count  = model.meshes[mesh_index].index_count;

    std::vector<Vertex> positions(model.positions.begin() + vertx_offset, model.positions.begin() + vertx_offset + vertex_count);
    std::vector<uint32_t> indices(model.indices.begin() + index_offset, model.indices.begin() + index_offset + index_count);

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
                                                       &positions[0].position.x,
                                                       positions.size(),
                                                       sizeof(Vertex),
                                                       max_vertices,
                                                       max_triangles,
                                                       .7f);

    const meshopt_Meshlet &last = meshlets[meshlet_count - 1];
    meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
    meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
    meshlets.resize(meshlet_count);


    classify_triangle_group(indices,meshlets,meshlet_vertices,meshlet_triangles);
    while (meshlets.size() % 32 != 0)
        meshlets.push_back(meshopt_Meshlet());

    for (const auto &m : meshlets)
    {
        auto bound = meshopt_Bounds(meshopt_computeMeshletBounds(&meshlet_vertices[m.vertex_offset],
                                                                 &meshlet_triangles[m.triangle_offset],
                                                                 m.triangle_count, &positions[0].position.x,
                                                                 positions.size(),
                                                                 sizeof(Vertex)));

        MeshletBound meshletBound{};
        meshletBound.center      = glm::vec4(bound.center[0], bound.center[1], bound.center[2], 1);
        meshletBound.cone_apex   = glm::vec4(bound.cone_apex[0], bound.cone_apex[1], bound.cone_apex[2], 0.f);
        meshletBound.cone_axis   = glm::vec4(bound.cone_axis_s8[0], bound.cone_axis_s8[1], bound.cone_axis_s8[2], 1.f);
        meshletBound.cone_cutoff = glm::vec4(bound.cone_cutoff_s8, bound.radius, 1, 1.1);

        bounds.push_back(meshletBound);
    }

    model.meshes[mesh_index].meshlet_vertex_offset   = canellaMeshlet.meshlet_vertices.size();
    model.meshes[mesh_index].meshlet_triangle_offset = canellaMeshlet.meshlet_triangles.size();
    model.meshes[mesh_index].meshlet_count           = meshlet_count;
    model.meshes[mesh_index].meshlet_offset          = canellaMeshlet.meshlets.size();

    canellaMeshlet.meshlets.insert(canellaMeshlet.meshlets.end(), meshlets.begin(), meshlets.end());
    canellaMeshlet.meshlet_triangles.insert(canellaMeshlet.meshlet_triangles.end(), meshlet_triangles.begin(), meshlet_triangles.end());
    canellaMeshlet.meshlet_vertices.insert(canellaMeshlet.meshlet_vertices.end(), meshlet_vertices.begin(), meshlet_vertices.end());
    canellaMeshlet.bounds.insert(canellaMeshlet.bounds.end(), bounds.begin(), bounds.end());
}

void
MeshProcessing::classify_triangle_group(std::vector<unsigned int> indices,
                                        std::vector<meshopt_Meshlet>& meshlets,
                                        std::vector<unsigned int> meshlet_vertices,
                                        std::vector<unsigned char> meshlet_triangles)
{
    //Create the MeshProcessing object for the cluster
    auto myMesh = MeshProcessing::Mesh(indices);
    std::map<uint32_t,uint32_t> counter;

    auto contains_vertex = [&](uint32_t index,Face& face)
    {
        for(auto& vertex : face.vertices)
        {
            if(vertex.id == index)
                return true;
        }
        return false;
    };

    for(int c = 0 ; c < meshlets.size(); c++)
    {
        auto& meshlet = meshlets[c];
        auto local_vertices = std::vector<unsigned int>(meshlet_vertices.begin() + meshlet.vertex_offset,meshlet_vertices.begin() + meshlet.vertex_offset + meshlet.vertex_count);
        auto local_tris = std::vector<unsigned char>(meshlet_triangles.begin() + meshlet.triangle_offset,meshlet_triangles.begin() + meshlet.triangle_offset + meshlet.triangle_count *3);

        std::vector<uint32_t> cluster_indices;


        //Get cluster indices inside the mesh
        cluster_indices.resize(local_tris.size());
        auto index = 0;
        for(auto& tri: local_tris)
        {
            cluster_indices[index] = local_vertices[tri];
            index++;
        }

        for(auto i = 0 ; i < cluster_indices.size(); i+= 3)
        {
            for(auto j = 0 ; j < myMesh.triangles.size(); ++j)
            {
                //test if cluster triangle is contained in each face
                auto& triangle = myMesh.triangles[j];
                if((contains_vertex(cluster_indices[i],triangle))
                && (contains_vertex(cluster_indices[i + 1],triangle))
                & (contains_vertex(cluster_indices[i + 2],triangle)))
                {
                    triangle.id = c;
                }
            }
        }
    }

    std::map<std::pair<int,int>,int>adjacency;
    for(auto& tri1 : myMesh.triangles)
    {
        for(auto& tri2 :myMesh.triangles)
        {
            auto shared_edge = 0;
            if(tri1.id == tri2.id )continue;
            shared_edge = contains_vertex(tri1.vertices[0].id,tri2)? shared_edge +1: shared_edge;
            shared_edge = contains_vertex(tri1.vertices[1].id,tri2)? shared_edge + 1: shared_edge;
            shared_edge = contains_vertex(tri1.vertices[2].id,tri2)? shared_edge + 1: shared_edge;
            if(shared_edge >=2)
                adjacency[std::pair(tri1.id,tri2.id)] ++;
        }
    }
    adjacency;
    Logger::Info("%d ",counter[0]);
}
