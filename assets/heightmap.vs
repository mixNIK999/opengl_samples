#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec2 in_texcoord;

struct vx_output_t
{
    vec3 wPos;
    vec3 normal;
    vec2 texcoord;
};
out vx_output_t v_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D u_heightmap;
uniform float u_height_scale;

void main()
{
    float color = texture2D(u_heightmap, in_texcoord.xy).r / 255;
    vec3 pos = in_position + color * in_norm * u_height_scale;
//    vec3 pos = in_position;

    mat4 mvp = projection * view * model;
    gl_Position = mvp * vec4(pos, 1.0);

    v_out.wPos = (model * vec4(pos, 1.0)).xyz;
    v_out.normal = (model * vec4(in_norm, 0)).xyz;
    v_out.texcoord = in_texcoord;
}