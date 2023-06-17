#ifndef CANELLA_SHADER_COMMOM
#define CANELLA_SHADER_COMMOM

struct MeshDrawInstance{
  mat4 model;
};

struct Meshlet
{
 /* offsets within meshlet_vertices and meshlet_triangles arrays with meshlet data */
	uint vertex_offset;
	uint triangle_offset;

	/* number of vertices and triangles used in the meshlet; data is stored in consecutive range defined by offset and count */
 	uint vertex_count;
	uint triangle_count;
};

struct MeshletBound{

	/* normal cone, useful for backface culling */
	vec4 cone_apex;
	vec4 cone_axis;
	vec4 cone_cutoff;
	vec4 center;
};

struct FrustumCamera
{
	vec3 camera_right;
	vec3 camera_front;
	vec3 camera_up;
	vec3 camera_pos;
	vec3 near_far_fovy;
};

//Todo break this command into multiple smaller structures (VERY IMPORTANT)
struct IndirectDrawDispatch{
  uint groupCountX;
  uint groupCountY;
  uint groupCountZ;
  uint vertex_offset;
  uint meshlet_offset;
  uint meshlet_triangle_offset;
  uint meshlet_vertex_offset;
  uint index_offset;
  uint mesh_id;
  uint meshlet_count;
  uint cx;
  uint cy;
  uint cz;
  uint radius;
};

struct Vertex
{
	vec4 position;
};

vec3 rotateQuat(vec3 v, vec4 q)
{
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

#endif