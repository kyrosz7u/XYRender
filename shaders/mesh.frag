#version 310 es

#extension GL_GOOGLE_include_directive : enable

layout(set=1, binding=0) uniform sampler2D base_color_texture_sampler;

layout(location=0) in highp vec3 world_pos;
layout(location=1) in highp vec3 normal;
layout(location=2) in highp vec4 tangent;
layout(location=3) in highp vec2 texcoord;

layout(location = 0) out highp vec4 out_color;

void main()
{
    out_color = texture(base_color_texture_sampler, texcoord);
}
