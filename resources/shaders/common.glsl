#ifndef CANELLA_SHADER_COMMOM
#define CANELLA_SHADER_COMMOM

struct MeshDrawInstance{
  mat4 model;
};

struct Meshlet
{
    vec3    center;
    float   radius;

    uint  cone_axis[3];
    uint  cone_cutoff;

    uint    data_offset;
    uint    mesh_index;
    uint    vertex_count;
    uint    triangle_count;
};


#endif