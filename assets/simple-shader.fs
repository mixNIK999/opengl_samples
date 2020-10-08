#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
    vec3 st;
};

in vx_output_t v_out;

uniform samplerCube u_tex;

void main()
{
    vec3 texture = texture(u_tex, v_out.st).rgb;

    o_frag_color = vec4(texture, 1.0);
}
