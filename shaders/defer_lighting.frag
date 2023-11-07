#version 450

#define m_max_direction_light_count 16
#define m_max_cascade_count 4
#define depth_bias 0.005

struct DirectionalLight
{
    highp vec4 color;
    highp vec3 direction;
    highp float intensity;
    highp int cascade_count;
};

struct ShadowMapSampleData
{
    highp mat4 light_view_proj;
    highp vec4 light_frustum_spere;
};

layout (set = 0, binding = 0, row_major) uniform _per_frame_ubo_data
{
    highp mat4 camera_proj_view;
    highp vec3 camera_pos;
    highp int directional_light_number;
    highp int cmdBuffer_index;
};

layout (set = 0, binding = 1) uniform _directional_light
{
    DirectionalLight directional_light[m_max_direction_light_count];
};

layout (set = 0, binding = 2, row_major) uniform _directional_light_projection
{
    ShadowMapSampleData shadowmap_sample_data[m_max_direction_light_count*m_max_cascade_count];
};

layout (input_attachment_index = 0, set = 1, binding = 0) uniform highp subpassInput gbuffer_color;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform highp subpassInput gbuffer_normal;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform highp subpassInput gbuffer_position;

layout (set = 2, binding = 0) uniform highp sampler2DArray directional_light_shadowmap_array;

layout(push_constant) uniform PushConsts {
    int cmdBuffer_index;
} pushConsts;

layout (location = 0) out highp vec4 out_color;

highp vec3 DecodeNormal(highp vec3 enc)
{
    return enc * 2.0f - 1.0f;
}

highp float SampleShadowMap(highp vec3 light_space_pos, highp int light_index)
{
    if(light_space_pos.z <= 0.0f || light_space_pos.z >= 1.0f)
    {
        return 1.0f;
    }

    highp vec3 sample_pos = vec3(light_space_pos.xy, float(light_index));
    highp float light_space_depth = texture(directional_light_shadowmap_array, sample_pos).r;

//    return light_space_pos.z - light_space_depth;

    // 可以考虑不要用imageArray，然后使用sampler2DShadow优化，避免手动比较
    // 返回可见性
    if (light_space_depth < light_space_pos.z - 0.02)
    {
        return 0.0f;
    }
    else
    {
        return 1.0f;
    }
}


highp float PCF(highp vec3 light_space_pos, highp int light_index)
{
    vec2 texelSize = 1.0 / (textureSize(directional_light_shadowmap_array, 0)).xy;

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            highp vec3 light_space_pos_uvw = vec3(light_space_pos.xy + vec2(x, y) * texelSize, float(light_index));
            highp float visibility = SampleShadowMap(light_space_pos_uvw, light_index);

            shadow += visibility;
        }
    }

    return 1.0f - shadow / 9.0f;
}


highp float hard_shadow(highp vec3 light_space_pos, highp int light_index)
{
    return SampleShadowMap(light_space_pos, light_index);
}

highp float GetCascadeShadow(highp vec3 world_pos, highp int light_index, highp int cascade_count)
{
    int i;
    for(i=0; i<cascade_count; ++i)
    {
        vec3 center = shadowmap_sample_data[light_index*m_max_cascade_count+i].light_frustum_spere.xyz;
        float radius = shadowmap_sample_data[light_index*m_max_cascade_count+i].light_frustum_spere.w;
        float dist = length(world_pos-center);
        if(dist < radius)
        {
            break;
        }
    }

    if(i == cascade_count)
    {
        return 1.0f;
    }

    highp vec4 light_space_pos = shadowmap_sample_data[light_index*m_max_cascade_count+i].light_view_proj * vec4(world_pos, 1.0);
    highp vec3 light_space_pos_uvz = light_space_pos.xyz / light_space_pos.w;

//    return light_space_pos_uvz.z;
    return hard_shadow(light_space_pos_uvz, light_index);
}

void main()
{
    vec3 color = subpassLoad(gbuffer_color).xyz;
    vec3 normal = DecodeNormal(subpassLoad(gbuffer_normal).xyz);
    vec3 position = subpassLoad(gbuffer_position).xyz;

    highp vec3 ambient_color = 0.2*color;
    highp vec3 diffuse_color = vec3(0.0, 0.0, 0.0);
    highp vec3 specular_color = vec3(0.0, 0.0, 0.0);

    highp float visibility;
    for (highp int i = 0; i < directional_light_number; ++i)
    {
        DirectionalLight light = directional_light[i];
        highp vec3 light_dir = normalize(light.direction);
        highp vec3 view_dir = normalize(camera_pos - position);
        highp vec3 half_dir = normalize(light_dir + view_dir);
        highp float NdotL = max(dot(normalize(normal), light_dir), 0.0);

//        int cascade_index = 2;
//
//        highp vec4 light_space_pos = shadowmap_sample_data[i*m_max_cascade_count+cascade_index].light_view_proj * vec4(position, 1.0);
//        highp vec3 light_space_pos_uvz = light_space_pos.xyz / light_space_pos.w;

//        highp float rvalue = 1.0f;
//        highp float gvalue = 0;
//
//        if(light_space_pos_uvz.z < 0.0f || light_space_pos_uvz.z > 1.0f)
//        {
//            rvalue = 0.0f;
//        }
//
//        if(light_space_pos_uvz.y > 0.5f && light_space_pos_uvz.y < 1.0f)
//        {
//            gvalue = 1.0f;
//        }
//
//        out_color = vec4(rvalue, gvalue, 0.0f, 1.0f);
//        return ;

//        if (NdotL <= 0.0)
//        {
//            visibility = 0.0f;
//        }
//        else
//        {
            visibility = GetCascadeShadow(position, i, light.cascade_count);
//        }

        diffuse_color += 0.6*visibility*color *light.color.xyz * light.intensity* NdotL;
        specular_color += 0.4*visibility*light.color.xyz*light.intensity * pow(max(dot(normalize(normal), half_dir), 0.0), 8.0);
    }

    diffuse_color /= float(directional_light_number);
    specular_color/=float(directional_light_number);

//    out_color = vec4(ambient_color+diffuse_color+specular_color, 1.0);
//    out_color = vec4(visibility/2.0f,0.0f,0.0f, 1.0);

    if(pushConsts.cmdBuffer_index != cmdBuffer_index)
    {
        out_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        out_color = vec4(ambient_color+diffuse_color+specular_color, 1.0);
    }
}