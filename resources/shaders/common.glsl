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
};


struct Vertex
{
	vec4 position;
};

#endif