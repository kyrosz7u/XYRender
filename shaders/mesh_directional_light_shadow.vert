#version 310 es

layout(set=0,binding=0,row_major) uniform _per_light_project_ubo_data
{
    mat4 project_matrix;
};

layout(set=0,binding=1,row_major) uniform _per_object_ubo_data
{
    mat4 model_matrix;
};

layout(location=0) in vec3 in_position;

void main()
{
    gl_Position = project_matrix * model_matrix * vec4(in_position,1.0);
}