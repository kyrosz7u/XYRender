#version 310 es

layout(set=0,binding = 0,row_major) uniform _per_frame_ubo_data
{
    mat4 viewProj;
    vec3 cameraPos;
    vec3 dirLightDir;
};

layout(location = 0) out vec3 out_UVW;

void main()
{
    const vec3 cube_corner_vertex_offsets[8] = vec3[8](vec3(1.0, 1.0, 1.0),
                                                       vec3(1.0, 1.0, -1.0),
                                                       vec3(1.0, -1.0, -1.0),
                                                       vec3(1.0, -1.0, 1.0),
                                                       vec3(-1.0, 1.0, 1.0),
                                                       vec3(-1.0, 1.0, -1.0),
                                                       vec3(-1.0, -1.0, -1.0),
                                                       vec3(-1.0, -1.0, 1.0));

    const int cube_triangle_index[36] = int[36](0, 1, 2, 2,
                                                3, 0, 4, 5,
                                                1, 1, 0, 4,
                                                7, 6, 5, 5,
                                                4, 7, 3, 2,
                                                6, 6, 7, 3,
                                                4, 0, 3, 3,
                                                7, 4, 1, 5,
                                                6, 6, 2, 1); // x+, y+, x-, y-, z+, z-

    out_UVW = cube_corner_vertex_offsets[cube_triangle_index[gl_VertexIndex]];
    vec4 cull_pos = viewProj * vec4(cameraPos + out_UVW, 1.0f);
    gl_Position = cull_pos.xyww;

}