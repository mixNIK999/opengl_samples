#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
    vec3 wPos;
    vec3 normal;
    vec2 texcoord;
};

in vx_output_t v_out;

uniform samplerCube u_tex;
uniform vec3 camera_pos;
uniform float ratio;

void main()
{
    vec3 normal = normalize(v_out.normal);

    vec3 toObj = normalize(v_out.wPos - camera_pos);
    vec3 toSky = refract(toObj, normal, ratio);
    vec3 texture = texture(u_tex, toSky).rgb;

    o_frag_color = vec4(texture, 1.0);
}
