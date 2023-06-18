#version 310 es

#extension GL_GOOGLE_include_directive: enable

#define m_max_direction_light_count 16

struct DirectionalLight
{
    highp vec4 color;
    highp vec3 direction;
    highp float intensity;
};

layout (set = 0, binding = 0, row_major) uniform _per_frame_ubo_data
{
    highp mat4 camera_proj_view;
    highp vec3 camera_pos;
    highp int directional_light_number;
};

layout (set = 0, binding = 2) uniform _directional_light
{
    DirectionalLight directional_light[m_max_direction_light_count];
};

layout (set = 0, binding = 3, row_major) uniform _directional_light_projection
{
    highp mat4 directional_light_proj[m_max_direction_light_count];
};


layout (set = 1, binding = 0) uniform sampler2D base_color_texture_sampler;

layout (set = 2, binding = 0) uniform highp sampler2DArray directional_light_shadowmap_array;

layout (location = 0) in highp vec3 world_pos;
layout (location = 1) in highp vec3 normal;
layout (location = 2) in highp vec4 tangent;
layout (location = 3) in highp vec2 texcoord;

layout (location = 0) out highp vec4 out_color;

highp float calculate_visibility(highp vec3 world_pos, highp int light_index)
{
    highp vec4 light_space_pos = directional_light_proj[light_index] * vec4(world_pos, 1.0);
    highp vec3 light_space_pos_ndc = light_space_pos.xyz / light_space_pos.w;

    if(light_space_pos_ndc.z >= 1.0f)
    {
        return 1.0f;
    }

    highp vec3 light_space_pos_uv = light_space_pos_ndc * 0.5 + 0.5;
    highp vec3 light_space_pos_uv_y_inverted = vec3(light_space_pos_uv.x, light_space_pos_uv.y, float(light_index));
    highp float light_space_depth = texture(directional_light_shadowmap_array, light_space_pos_uv_y_inverted).r;
    if(light_space_depth < light_space_pos_ndc.z)
    {
        return 0.0f;
    }
    else
    {
        return 1.0f;
    }
}

void main()
{
    highp vec4 diffuse_texture = texture(base_color_texture_sampler, texcoord);

    highp vec3 ambient_color = 0.2*diffuse_texture.xyz;
    highp vec3 diffuse_color = vec3(0.0, 0.0, 0.0);
    highp vec3 specular_color = vec3(0.0, 0.0, 0.0);


    for (highp int i = 0; i < directional_light_number; ++i)
    {
        DirectionalLight light = directional_light[i];
        highp vec3 light_dir = normalize(light.direction);
        highp vec3 view_dir = normalize(camera_pos - world_pos);
        highp vec3 half_dir = normalize(light_dir + view_dir);
        highp float NdotL = max(dot(normalize(normal), light_dir), 0.0);

        highp float visibility;
        if(NdotL <= 0.0)
        {
            visibility = 0.0f;
        }
        else
        {
            visibility = calculate_visibility(world_pos, i);
        }

        diffuse_color += 0.6*visibility*diffuse_texture.xyz *light.color.xyz * light.intensity* NdotL;
        specular_color += 0.4*visibility*light.color.xyz*light.intensity * pow(max(dot(normalize(normal), half_dir), 0.0), 8.0);
    }

    diffuse_color /= float(directional_light_number);
    specular_color/=float(directional_light_number);

    out_color = vec4(ambient_color+diffuse_color+specular_color, 1.0);
}
