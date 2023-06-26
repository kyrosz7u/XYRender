#version 450

layout (set = 1, binding = 0) uniform sampler2D base_color_texture_sampler;

layout (location = 0) in highp vec3 world_pos;
layout (location = 1) in highp vec3 normal;
layout (location = 2) in highp vec4 tangent;
layout (location = 3) in highp vec2 texcoord;

layout (location = 0) out highp vec4 gbuffer_color;
layout (location = 1) out highp vec4 gbuffer_normal;
layout (location = 2) out highp vec4 gbuffer_position;

highp vec3 NormalEncode(highp vec3 n)
{
    return n * 0.5 + 0.5;
}

void main()
{
    highp vec4 texture_color = texture(base_color_texture_sampler, texcoord);

    gbuffer_normal.xyz = NormalEncode(normalize(normal));
    gbuffer_position.xyz = world_pos.xyz;
    gbuffer_color.rgb = texture_color.rgb;
}

