#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform SpriteUniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
  vec4 frame_pos_xy_direction_z;
} ubo;
layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 in_tex_coord;

layout(location = 0) out vec4 out_frag_color;
layout(location = 1) out vec4 out_normal_color;

void main() {
  // Check for direction facing
  if(ubo.frame_pos_xy_direction_z.z > 0)
    out_frag_color = texture(tex_sampler, in_tex_coord + ubo.frame_pos_xy_direction_z.xy);
  else
    out_frag_color = texture(tex_sampler, vec2(ubo.frame_pos_xy_direction_z.x - in_tex_coord.x, in_tex_coord.y + ubo.frame_pos_xy_direction_z.y));

  out_normal_color = vec4(0.0, 0.0, 1.0, 1.0);  // Placeholder, pointing straight out of the screen in view space
}
