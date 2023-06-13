#version 310 es

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding = 0,row_major) uniform _per_frame_ubo_data
{
    mat4 viewProj;
    vec3 cameraPos;
    vec3 dirLightDir;
};

layout(set=0,binding=1,row_major) uniform _per_object_ubo_data
{
    mat4 model;
};

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec4 in_tangent;
layout(location=3) in vec2 in_texCoord;

layout(location=0) out vec3 world_pos;
layout(location=1) out vec3 normal;
layout(location=2) out vec4 tangent;
layout(location=3) out vec2 texcoord;

void main()
{
    world_pos = (model * vec4(in_position,1.0)).xyz;
    float model_a = length(model[0].xyz);
    float model_b = length(model[1].xyz);
    float model_c = length(model[2].xyz);
    vec3 model_x = model[0].xyz/model_a/model_a;
    vec3 model_y = model[1].xyz/model_b/model_b;
    vec3 model_z = model[2].xyz/model_c/model_c;

    mat3x3 normal_matrix = mat3x3(model_x,model_y,model_z);

    normal = normal_matrix*in_normal;
    tangent = model*in_tangent;
    texcoord = in_texCoord;

    gl_Position =  viewProj*model * vec4(in_position,1.0);
}
