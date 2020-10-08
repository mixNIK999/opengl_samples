#version 330 core

layout (location = 0) in vec3 in_position;

struct vx_output_t
{
    vec3 st;
};
out vx_output_t v_out;

uniform mat4 u_mvp;

void main()
{
    v_out.st = in_position;

    gl_Position = u_mvp * vec4(in_position, 1.0);

    //gl_Position.xyz *= 0;

    //gl_Position = u_mvp * vec4(3 * rotated_pos.x, 2 * rotated_pos.y, in_position.z, 1.0);
    //gl_Position /= gl_Position.w;
}
