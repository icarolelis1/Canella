#ifndef CANELLA_MESH
#define CANELLA_MESH
#include "Render/Render.h"
#include <unordered_map>

namespace Canella{

    namespace MeshProcessing
    {
        struct Face;
        struct HalfEdge
        {
            HalfEdge * next;
            HalfEdge * twin;
            Face     * triangle;
        };

        struct VertexHe
        {
            uint32_t id;
            HalfEdge * he;
        };

        struct Face{
            HalfEdge* he;
            uint32_t id;
            std::array<VertexHe,3> vertices;
            Face(uint32_t _id):id(_id){};
            Face() = default;
            ~Face() = default;
        };

        struct Edge
        {
            glm::vec4 start;
            glm::vec4 end;
            HalfEdge* he;
        };

        class Mesh
        {
        public:
            Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
            std::map<std::pair<uint32_t ,uint32_t>, HalfEdge* > edges;
            std::vector<VertexHe> vertices;
            std::vector<Face>   triangles;
            std::vector<Edge>   geo_edges;
            std::vector<Mesh>   meshes;
            glm::mat4          *model_matrix;
            Meshlet             meshlet_compositions;

            ~Mesh();
            void classify_triangles();
        private:

        };

        std::array<glm::vec2,2> project_box_from_sphere(glm::mat4* model_matrix,SphereBoundingVolume& volume,int width,int height,glm::mat4& view,glm::mat4& projection);
        using cluster_adjacency_edges = std::map<std::pair<int,int>,int>;
        void classify_triangle_group(std::vector<Vertex>&vertices,
                                     std::vector<unsigned int>& indices,
                                     std::vector<meshopt_Meshlet>& meshlets,
                                     std::vector<unsigned int>& meshlet_vertices,
                                     std::vector<unsigned char>& meshlet_triangles);

        void simplify_indices(std::vector<Vertex>& vertices,std::vector<uint32_t>& indices);
        int count_boundaries(MeshProcessing::Mesh& mesh);
        void build_meshlets(Canella::Meshlet &canellaMeshlet, Canella::ModelMesh &model, int mesh_index);
        void load_asset_mesh(ModelMesh &mesh, const ::std::string &assetsPath, const std::string &source);
        void load_instance_data(ModelMesh &mesh, const ::std::string &assetsPath, const std::string &instance);
    };
}

#endif