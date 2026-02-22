#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mana/core/corecommon.h"
#include "mana/utilities/jsmn.h"

struct AnimationChannel {
  int target_node;
  char* path;
};

struct AnimationSampler {
  int input;
  char* interpolation;
  int output;
};

// Define struct to store glTF data
struct GLTF {
  int num_meshes;
  int* mesh_materials;
  int* mesh_indices;
  float* mesh_positions;
  float* mesh_normals;
  float* mesh_uvs;

  int num_materials;
  float* material_diffuse;
  float* material_specular;
  float* material_shininess;
  float* material_ambient;

  int num_skins;
  int* skin_joints;
  float* skin_inverseBindMatrices;
  float* skin_bindShapeMatrix;

  int num_animations;
  struct AnimationChannel* animation_channels;
  struct AnimationSampler* animation_samplers;
};
