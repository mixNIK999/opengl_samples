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
uniform float F0;

void main()
{
    vec3 normal = normalize(v_out.normal);

    vec3 toObj = normalize(v_out.wPos - camera_pos);
    vec3 refl = reflect(toObj, normal);
    vec3 refr = refract(toObj, normal, ratio);

    vec3 vec_F0 = vec3(F0);
    vec3 F = clamp(vec_F0 + (1 - vec_F0) * pow(1 - dot(toObj, normal), 5), 0, 1);

    vec3 refl_texture = texture(u_tex, refl).rgb;
    vec3 refr_texture = texture(u_tex, refr).rgb;

    o_frag_color = vec4(mix(refl_texture, refr_texture, F), 1.0);
}
