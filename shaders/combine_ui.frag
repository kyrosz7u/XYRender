#version 310 es

layout (input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput scene_input_color;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform highp subpassInput ui_output_color;

layout (location = 0) out highp vec4 color;

void main()
{
    highp vec4 scene_color = subpassLoad(scene_input_color);
    highp vec4 ui_color = subpassLoad(ui_output_color);

    highp float blend_weight = ui_color.a;

    color = mix(scene_color, ui_color, blend_weight);
}