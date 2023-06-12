#version 310 es

layout(set = 0, binding = 1) uniform samplerCube cubemap_sampler;

layout(location = 0) in highp vec3 in_UVW;

layout(location = 0) out highp vec4 out_color;

void main()
{
    out_color = texture(cubemap_sampler, in_UVW);
}