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

void main()
{
    vec3 normal = normalize(v_out.normal);

//    vec3 vec_x = dFdx(v_out.wPos);
//    vec3 vec_y = dFdy(v_out.wPos);
//
//    vec3 normal = normalize(cross(vec_x, vec_y));

    vec3 toObj = normalize(v_out.wPos - camera_pos);
    vec3 toSky = reflect(toObj, normal);
    vec3 texture = texture(u_tex, toSky).rgb;

    o_frag_color = vec4(texture, 1.0);
    //  o_frag_color = vec4(0.5, 0.5, 0.5, 1.0);
}
