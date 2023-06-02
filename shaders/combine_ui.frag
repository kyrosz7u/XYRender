#version 310 es

layout (input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput input_color;
layout (input_attachment_index = 0, set = 0, binding = 1) uniform highp subpassInput ui_output_color;

layout (location = 0) out highp vec4 color;

void main()
{
    highp vec4 input_color = subpassLoad(input_color);
    highp vec4 ui_color = subpassLoad(ui_output_color);

    highp float blend_weight = ui_color.a;

    highp vec4 out_color = mix(input_color, ui_color, blend_weight);

//    color = vec4(out_color.rgb,1.0f);
    color = ui_color;
//    if (ui_color.r < 1e-6 && ui_color.g < 1e-6 && ui_color.b < 1e-6)
//    {
//        color = input_color;
//    }
//    else
//    {
//        color = ui_color;
//    }
}