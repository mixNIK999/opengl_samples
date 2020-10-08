#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
  vec3 normal;
  vec2 texcoord;
};

in vx_output_t v_out;

uniform sampler2D u_tex;

void main()
{
  vec3 texture = texture(u_tex, v_out.texcoord.xy).rgb;

  o_frag_color = vec4(texture, 1.0);
//  o_frag_color = vec4(0.5, 0.5, 0.5, 1.0);
}
