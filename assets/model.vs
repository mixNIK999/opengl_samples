#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec2 in_texcoord;

struct vx_output_t
{
    vec3 normal;
    vec2 texcoord;
};
out vx_output_t v_out;

uniform mat4 u_mvp;

void main()
{
    gl_Position = u_mvp * vec4(in_position, 1.0);
    v_out.normal = (u_mvp * vec4(in_norm, 0)).xyz;
    v_out.texcoord = in_texcoord;
}
