#pragma once

#include <mana/graphics/apis/api.h>

#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/gbuffer/gbuffer.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/graphics/utilities/texture/texture.h"
#include "mana/utilities/xmlparser.h"


#define MAX_JOINTS 50

struct ModelUniformBufferObject {
  alignas(16) mat4 model;
  alignas(16) mat4 view;
  alignas(16) mat4 proj;
  alignas(16) vec3 camera_pos;
};

struct ModelAnimationUniformBufferObject {
  alignas(16) mat4 joint_transforms[MAX_JOINTS];
};

struct RawVertexModel {
  vec3 position;
  int32_t texture_index;
  int32_t normal_index;
  int32_t color_index;
  struct RawVertexModel *duplicate_vertex;
  uint32_t index;
  float length;
  struct VertexSkinData *weights_data;
};

struct ModelData {
  struct Vector *vertices;
  struct Vector *tex_coords;
  struct Vector *normals;
  struct Vector *colors;
  struct Vector *indices;
  struct Vector *joint_ids;
  struct Vector *vertex_weights;
};

struct KeyFrame {
  float time_step;
  struct Map *pose;
};

struct Animation {
  float length;
  struct ArrayList *key_frames;
};

struct JointTransform {
  vec3 position;
  quat rotation;
};

struct Animator {
  struct ModelCommon *entity;
  struct Animation *current_animation;
  float animation_time;
};

struct AnimationData {
  float length_seconds;
  struct ArrayList *key_frames;
};

struct KeyFrameData {
  float time;
  struct ArrayList *joint_transforms;
};

struct JointTransformData {
  char *joint_name_id;
  mat4 joint_local_transform;
};

struct ModelJoint {
  int32_t index;
  char *name;
  struct ArrayList *children;
  mat4 animation_transform;
  mat4 local_bind_transform;
  mat4 inverse_bind_transform;
};

struct VertexSkinData {
  struct Vector *joint_ids;
  struct Vector *weights;
};

struct SkinningData {
  struct Vector *joint_order;
  struct Vector *vertices_skin_data;
};

struct JointData {
  int32_t index;
  char *name_id;
  mat4 bind_local_transform;
  struct ArrayList *children;
};

struct SkeletonData {
  int32_t joint_count;
  struct JointData *head_joint;
};

enum {
  MODEL_SUCCESS = 1
};

static inline void joint_init(struct ModelJoint *joint, int32_t index, char *name, mat4 bind_local_transform) {
  joint->animation_transform = MAT4_IDENTITY;
  joint->inverse_bind_transform = MAT4_IDENTITY;
  joint->children = malloc(sizeof(struct ArrayList));
  array_list_init(joint->children);
  joint->index = index;
  joint->name = _strdup(name);
  joint->local_bind_transform = bind_local_transform;
}

static inline void joint_calc_inverse_bind_transform(struct ModelJoint *joint, mat4 parent_bind_transform) {
  mat4 bind_transform = MAT4_ZERO;
  bind_transform = mat4_mul(parent_bind_transform, joint->local_bind_transform);
  joint->inverse_bind_transform = mat4_inverse(bind_transform);
  for (size_t child_num = 0; child_num < array_list_size(joint->children); child_num++) {
    struct ModelJoint *child_joint = (struct ModelJoint *)array_list_get(joint->children, child_num);
    joint_calc_inverse_bind_transform(child_joint, bind_transform);
  }
}

static inline struct ModelJoint *model_create_joints(struct JointData *root_joint_data) {
  struct ModelJoint *joint = malloc(sizeof(struct ModelJoint));
  joint_init(joint, root_joint_data->index, root_joint_data->name_id, root_joint_data->bind_local_transform);

  for (size_t joint_child_num = 0; joint_child_num < array_list_size(root_joint_data->children); joint_child_num++) {
    struct JointData *child_joint = (struct JointData *)array_list_get(root_joint_data->children, joint_child_num);
    array_list_add(joint->children, model_create_joints(child_joint));
  }
  return joint;
}

static inline void model_delete_joints(struct ModelJoint *joint) {
  if (joint->children != NULL && !array_list_empty(joint->children)) {
    for (size_t joint_num = 0; joint_num < array_list_size(joint->children); joint_num++)
      model_delete_joints((struct ModelJoint *)array_list_get(joint->children, joint_num));
  }
  free(joint->name);
  array_list_delete(joint->children);
  free(joint->children);
  free(joint);
}

static inline void model_delete_joints_data(struct JointData *joint_data) {
  if (joint_data->children != NULL && !array_list_empty(joint_data->children)) {
    for (size_t joint_num = 0; joint_num < array_list_size(joint_data->children); joint_num++)
      model_delete_joints_data((struct JointData *)array_list_get(joint_data->children, joint_num));
  }
  free(joint_data->name_id);
  array_list_delete(joint_data->children);
  free(joint_data->children);
  free(joint_data);
}

static inline void model_delete_animation(struct Animation *animation) {
  // Think memory bug is in here
  for (size_t frame_num = 0; frame_num < array_list_size(animation->key_frames); frame_num++) {
    struct KeyFrame *key_frame = (struct KeyFrame *)array_list_get(animation->key_frames, frame_num);
    map_delete(key_frame->pose);
    free(key_frame->pose);
    free(key_frame);
  }

  array_list_delete(animation->key_frames);
  free(animation->key_frames);
}

static inline void key_frame_init(struct KeyFrame *key_frame, float time_stamp, struct Map *joint_key_frames) {
  key_frame->time_step = time_stamp;
  key_frame->pose = joint_key_frames;
}

static inline void animation_init(struct Animation *animation, float length_in_seconds, struct ArrayList *frames) {
  animation->key_frames = frames;
  animation->length = length_in_seconds;
}

static inline struct JointTransform model_create_transform(struct JointTransformData *data) {
  mat4 mat = data->joint_local_transform;
  vec3 translation = (vec3){.data[0] = mat.vecs[3].data[0], .data[1] = mat.vecs[3].data[1], .data[2] = mat.vecs[3].data[2]};
  quat rot = mat4_to_quaternion(mat);
  return (struct JointTransform){.position = translation, .rotation = rot};
}

static inline void model_get_joint_transforms(struct ModelJoint *head_joint, mat4 dest[MAX_JOINTS]) {
  dest[head_joint->index] = head_joint->animation_transform;
  for (size_t child_joint_num = 0; child_joint_num < array_list_size(head_joint->children); child_joint_num++)
    model_get_joint_transforms((struct ModelJoint *)array_list_get(head_joint->children, child_joint_num), dest);
}

static inline struct KeyFrame *model_create_key_frame(struct KeyFrameData *data) {
  struct Map *map = malloc(sizeof(struct Map));
  map_init(map, sizeof(struct JointTransform));
  for (size_t joint_num = 0; joint_num < array_list_size(data->joint_transforms); joint_num++) {
    struct JointTransformData *joint_data = (struct JointTransformData *)array_list_get(data->joint_transforms, joint_num);
    struct JointTransform joint_transform = model_create_transform(joint_data);
    map_set(map, joint_data->joint_name_id, &joint_transform);
  }
  struct KeyFrame *key_frame = malloc(sizeof(struct KeyFrame));
  key_frame_init(key_frame, data->time, map);
  return key_frame;
}

static inline struct ModelJoint *model_create_joints_clone(struct ModelJoint *root_joint) {
  struct ModelJoint *joint = malloc(sizeof(struct ModelJoint));
  *joint = *root_joint;
  joint->name = _strdup(root_joint->name);
  joint->children = malloc(sizeof(struct ArrayList));
  array_list_init(joint->children);

  for (size_t joint_child_num = 0; joint_child_num < array_list_size(root_joint->children); joint_child_num++)
    array_list_add(joint->children, model_create_joints_clone((struct ModelJoint *)array_list_get(root_joint->children, joint_child_num)));

  return joint;
}

static inline void model_joints_clone_delete(struct ModelJoint *root_joint) {
  if (root_joint->children != NULL && !array_list_empty(root_joint->children)) {
    for (size_t joint_num = 0; joint_num < array_list_size(root_joint->children); joint_num++)
      model_delete_joints((struct ModelJoint *)array_list_get(root_joint->children, joint_num));
  }
  free(root_joint->name);
  array_list_delete(root_joint->children);
  free(root_joint->children);
  free(root_joint);
}

struct ModelSettings {
  char *path;
  int32_t max_weights;
  struct Shader *shader;
  struct Texture *diffuse_texture;
  struct Texture *normal_texture;
  struct Texture *metallic_texture;
  struct Texture *roughness_texture;
  struct Texture *ao_texture;
};

#ifdef VULKAN_API_SUPPORTED
struct ModelVulkan {
  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;

  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;

  VkBuffer uniform_buffer;
  VkDeviceMemory uniform_buffers_memory;
  VkBuffer uniform_animation_buffer;
  VkDeviceMemory uniform_animation_buffers_memory;
  VkBuffer lighting_uniform_buffer;
  VkDeviceMemory lighting_uniform_buffers_memory;

  VkDescriptorSet *descriptor_set;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct ModelDirectX12 {
  ID3D12Resource *vertex_buffer;                // Vertex Buffer Resource
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;  // View into the Vertex Buffer
                                                //
  ID3D12Resource *index_buffer;                 // Index Buffer Resource
  D3D12_INDEX_BUFFER_VIEW index_buffer_view;    // View into the Index Buffer
                                                //
  ID3D12Resource *constant_buffer;              // Constant Buffer Resource
  ID3D12Resource *animation_constant_buffer;    // Constant Buffer Resource
  ID3D12Resource *lighting_constant_buffer;     // Constant Buffer Resource
};
#endif

struct ModelCommon {
  struct Shader *shader_handle;
  struct Mesh *model_mesh;
  struct Texture *model_diffuse_texture;
  struct Texture *model_normal_texture;
  struct Texture *model_metallic_texture;
  struct Texture *model_roughness_texture;
  struct Texture *model_ao_texture;
  struct SkeletonData *joints;
  struct ModelJoint *root_joint;
  struct Animator *animator;
  struct Animation *animation;
  bool animated;
  char *path;

  mat4 temp_transform;

  vec3 position;
  quat rotation;
  vec3 scale;

  size_t model_num;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct ModelVulkan model_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct ModelDirectX12 model_directx12;
#endif
  };
};
