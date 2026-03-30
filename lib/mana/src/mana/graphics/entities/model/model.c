#include "mana/graphics/entities/model/model.h"

static inline bool xml_node_has_children(struct XmlNode* node) {
  return node != NULL && node->child_nodes != NULL && node->child_nodes->num_buckets > 0;
}

uint_fast8_t model_init(struct Model* model, struct APICommon* api_common, struct ModelSettings* model_settings, size_t num) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    model->model_func = VULKAN_MODEL;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    model->model_func = directx_12_MODEL;
#endif

  struct XmlNode* collada_node = xml_parser_load_xml_file(model_settings->path);
  if (collada_node == NULL)
    return MODEL_ERROR;

  struct XmlNode* library_controllers_node = xml_node_get_child(collada_node, "library_controllers");
  struct XmlNode* library_geometries_node = xml_node_get_child(collada_node, "library_geometries");
  struct XmlNode* visual_scenes_node = xml_node_get_child(collada_node, "library_visual_scenes");
  struct XmlNode* anim_node = xml_node_get_child(collada_node, "library_animations");

  bool has_skin = xml_node_has_children(library_controllers_node);
  bool has_animation = xml_node_has_children(anim_node);

  /* For this engine, only treat as animated if it has both skin and animation data */
  model->model_common.animated = has_skin && has_animation;

  model->model_common.animator = NULL;
  model->model_common.animation = NULL;
  model->model_common.root_joint = NULL;
  model->model_common.joints = NULL;

  struct SkinningData* skinning_data = NULL;

  if (model->model_common.animated) {
    skinning_data = skin_loader_extract_skin_data(library_controllers_node, model_settings->max_weights);

    if (skinning_data == NULL) {
      xml_parser_delete(collada_node);
      return MODEL_ERROR;
    }

    model->model_common.joints =
        skeleton_loader_extract_bone_data(visual_scenes_node, skinning_data->joint_order, api_common->inverted_y);

    model->model_common.model_mesh =
        geometry_loader_extract_model_data(api_common, library_geometries_node, skinning_data->vertices_skin_data, model->model_common.animated, api_common->inverted_y);

    if (model->model_common.joints != NULL &&
        model->model_common.joints->head_joint != NULL) {
      model->model_common.root_joint =
          model_create_joints(model->model_common.joints->head_joint);

      if (model->model_common.root_joint != NULL)
        joint_calc_inverse_bind_transform(model->model_common.root_joint, MAT4_IDENTITY);
    }

    /* Animator can exist even if there is no animation clip */
    model->model_common.animator = (struct Animator*)malloc(sizeof(struct Animator));
    if (model->model_common.animator != NULL)
      animator_init(model->model_common.animator, &(model->model_common));

    /* Only try to load animation if library_animations exists */
    if (anim_node != NULL && visual_scenes_node != NULL) {
      struct AnimationData* animation_data =
          animation_extract_animation(anim_node, visual_scenes_node, api_common->inverted_y);

      if (animation_data != NULL &&
          animation_data->key_frames != NULL &&
          array_list_size(animation_data->key_frames) > 0) {
        struct ArrayList* frames = (struct ArrayList*)malloc(sizeof(struct ArrayList));
        if (frames != NULL) {
          array_list_init(frames);

          for (size_t frame_num = 0; frame_num < array_list_size(animation_data->key_frames); frame_num++) {
            array_list_add(
                frames,
                model_create_key_frame(
                    (struct KeyFrameData*)array_list_get(animation_data->key_frames, frame_num)));
          }

          model->model_common.animation = (struct Animation*)malloc(sizeof(struct Animation));
          if (model->model_common.animation != NULL) {
            animation_init(model->model_common.animation, animation_data->length_seconds, frames);

            if (model->model_common.animator != NULL)
              animator_do_animation(model->model_common.animator,
                                    model->model_common.animation);
          } else {
            array_list_delete(frames);
            free(frames);
          }
        }
      }

      /* cleanup animation_data if it was created */
      if (animation_data != NULL) {
        if (animation_data->key_frames != NULL) {
          for (size_t frame_num = 0; frame_num < array_list_size(animation_data->key_frames); frame_num++) {
            struct KeyFrameData* key_frame_data =
                (struct KeyFrameData*)array_list_get(animation_data->key_frames, frame_num);

            if (key_frame_data != NULL) {
              if (key_frame_data->joint_transforms != NULL) {
                for (size_t transform_num = 0; transform_num < array_list_size(key_frame_data->joint_transforms); transform_num++) {
                  struct JointTransformData* joint_transform_data =
                      (struct JointTransformData*)array_list_get(
                          key_frame_data->joint_transforms, transform_num);

                  if (joint_transform_data != NULL) {
                    free(joint_transform_data->joint_name_id);
                    free(joint_transform_data);
                  }
                }
                array_list_delete(key_frame_data->joint_transforms);
                free(key_frame_data->joint_transforms);
              }
              free(key_frame_data);
            }
          }
          array_list_delete(animation_data->key_frames);
          free(animation_data->key_frames);
        }
        free(animation_data);
      }
    }

    for (size_t joint_num = 0; joint_num < vector_size(skinning_data->joint_order); joint_num++)
      free(*((char**)vector_get(skinning_data->joint_order, joint_num)));

    vector_delete(skinning_data->joint_order);
    free(skinning_data->joint_order);

    for (size_t vertice_num = 0; vertice_num < vector_size(skinning_data->vertices_skin_data); vertice_num++) {
      struct VertexSkinData* vertex_skin_data = (struct VertexSkinData*)vector_get(skinning_data->vertices_skin_data, vertice_num);

      if (vertex_skin_data != NULL) {
        vector_delete(vertex_skin_data->joint_ids);
        free(vertex_skin_data->joint_ids);
        vector_delete(vertex_skin_data->weights);
        free(vertex_skin_data->weights);
      }
    }

    vector_delete(skinning_data->vertices_skin_data);
    free(skinning_data->vertices_skin_data);
    free(skinning_data);
  } else {
    model->model_common.model_mesh = geometry_loader_extract_model_data(api_common, library_geometries_node, NULL, model->model_common.animated, api_common->inverted_y);
  }

  model->model_common.path = _strdup(model_settings->path);
  model->model_common.model_diffuse_texture = model_settings->diffuse_texture;
  model->model_common.model_normal_texture = model_settings->normal_texture;
  model->model_common.model_metallic_texture = model_settings->metallic_texture;
  model->model_common.model_roughness_texture = model_settings->roughness_texture;
  model->model_common.model_ao_texture = model_settings->ao_texture;
  model->model_common.shader_handle = model_settings->shader;
  model->model_common.model_num = num;

  xml_parser_delete(collada_node);
  return MODEL_SUCCESS;
}

void model_delete(struct Model* model, struct APICommon* api_common) {
  if (model->model_common.animated) {
    if (model->model_common.joints != NULL) {
      if (model->model_common.joints->head_joint != NULL)
        model_delete_joints_data(model->model_common.joints->head_joint);
      free(model->model_common.joints);
    }

    if (model->model_common.root_joint != NULL)
      model_delete_joints(model->model_common.root_joint);

    if (model->model_common.animation != NULL) {
      model_delete_animation(model->model_common.animation);
      free(model->model_common.animation);
    }

    if (model->model_common.animator != NULL)
      free(model->model_common.animator);
  }

  free(model->model_common.path);
  mesh_delete(model->model_common.model_mesh, api_common);
  free(model->model_common.model_mesh);
}

void model_update_uniforms(struct Model* model, struct APICommon* api_common, struct GBuffer* gbuffer, vec3d position, vec4 light_pos, vec4 diffuse_color, vec4 ambient_color, vec4 specular_light) {
  model->model_func.model_update_uniforms(&model->model_common, api_common, gbuffer, position, light_pos, diffuse_color, ambient_color, specular_light);
}

struct Model* model_get_clone(struct Model* model, struct APICommon* api_common) {
  struct Model* new_model = (struct Model*)calloc(1, sizeof(struct Model));
  *new_model = *model;

  new_model->model_common.position = VEC3_ZERO;
  new_model->model_common.rotation = QUAT_DEFAULT;
  new_model->model_common.scale = VEC3_ONE;

  new_model->model_common.model_mesh = (struct Mesh*)calloc(1, sizeof(struct Mesh));
  new_model->model_common.model_mesh->mesh_common.indices = (struct Vector*)calloc(1, sizeof(struct Vector));
  new_model->model_common.model_mesh->mesh_common.vertices = (struct Vector*)calloc(1, sizeof(struct Vector));

  *new_model->model_common.model_mesh->mesh_common.indices = *model->model_common.model_mesh->mesh_common.indices;
  *new_model->model_common.model_mesh->mesh_common.vertices = *model->model_common.model_mesh->mesh_common.vertices;
  //(new_model->animated) ? mesh_model_init(new_model->model_mesh) : mesh_model_static_init(new_model->model_mesh);
  new_model->model_common.model_mesh->mesh_common.indices->items = (void*)calloc(1, model->model_common.model_mesh->mesh_common.indices->capacity * model->model_common.model_mesh->mesh_common.indices->memory_size);
  memcpy(new_model->model_common.model_mesh->mesh_common.indices->items, model->model_common.model_mesh->mesh_common.indices->items, model->model_common.model_mesh->mesh_common.indices->capacity * model->model_common.model_mesh->mesh_common.indices->memory_size);

  new_model->model_common.model_mesh->mesh_common.vertices->items = (void*)calloc(1, model->model_common.model_mesh->mesh_common.vertices->capacity * model->model_common.model_mesh->mesh_common.vertices->memory_size);
  memcpy(new_model->model_common.model_mesh->mesh_common.vertices->items, model->model_common.model_mesh->mesh_common.vertices->items, model->model_common.model_mesh->mesh_common.vertices->capacity * model->model_common.model_mesh->mesh_common.vertices->memory_size);

  new_model->model_common.model_mesh->mesh_common.mesh_type = model->model_common.model_mesh->mesh_common.mesh_type;
  new_model->model_common.model_mesh->mesh_common.mesh_memory_size = model->model_common.model_mesh->mesh_common.mesh_memory_size;

  if (new_model->model_common.animated) {
    new_model->model_common.root_joint = model_create_joints_clone(model->model_common.root_joint);
    new_model->model_common.animator = (struct Animator*)calloc(1, sizeof(struct Animator));
    animator_init(new_model->model_common.animator, &(new_model->model_common));
    animator_do_animation(new_model->model_common.animator, new_model->model_common.animation);
  }

  model->model_func.model_clone_init(&(new_model->model_common), api_common);

  return new_model;
}

void model_clone_delete(struct Model* model, struct APICommon* api_common) {
  model->model_func.model_clone_delete(&model->model_common, api_common);

  // TODO: Delete uniform colors if needed

  if (model->model_common.animated) {
    free(model->model_common.animator);
    model_joints_clone_delete(model->model_common.root_joint);
  }

  mesh_delete(model->model_common.model_mesh, api_common);
  free(model->model_common.model_mesh);
}

void model_render(struct Model* model, struct GBuffer* gbuffer, double delta_time) {
  if (model->model_common.animated && model->model_common.animator != NULL)
    animator_update(model->model_common.animator, delta_time);

  model->model_func.model_render(&(model->model_common), gbuffer, delta_time);
}

void model_recreate(struct Model* model, struct APICommon* api_common) {
  model->model_func.model_clone_delete(&model->model_common, api_common);
  model->model_func.model_clone_init(&model->model_common, api_common);
}

// TODO: Maybe get instanced/batched where specific vertex data is not needed
