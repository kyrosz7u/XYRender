#version 310 es

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding = 0) uniform _per_frame_ubo_data
{
    mat4 viewProj;
    vec3 cameraPos;
    vec3 dirLightDir;
};

layout(set=0,binding=1) uniform _per_object_ubo_data
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
    normal = in_normal;
    tangent = in_tangent;
    texcoord = in_texCoord;

    gl_Position = viewProj * model * vec4(in_position,1.0);
}
