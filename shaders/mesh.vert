#version 310 es

#extension GL_GOOGLE_include_directive : enable

// #include "constants.h"
// #include "structures.h"

struct DirectionalLight
{
    vec3  direction;
    float _padding_direction;
    vec3  color;
    float _padding_color;
};

layout(location = 0) in vec3 in_position; // for some types as dvec3 takes 2 locations
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_world_position; // output in framebuffer 0 for fragment shader
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec2 out_texcoord;

void main()
{
    highp vec3 model_position;
    highp vec3 model_normal;
    highp vec3 model_tangent;

    // TODO: normal matrix
    // mat3x3 tangent_matrix = mat3x3(model_matrix[0].xyz, model_matrix[1].xyz, model_matrix[2].xyz);
    // out_normal            = normalize(tangent_matrix * model_normal);
    // out_tangent           = normalize(tangent_matrix * model_tangent);

    out_texcoord = in_texcoord;

    gl_Position = vec4(0.0f,0.0f,0.0f, 1.0f);
}
