#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mana/core/corecommon.h"

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
  r32* mesh_positions;
  r32* mesh_normals;
  r32* mesh_uvs;

  int num_materials;
  r32* material_diffuse;
  r32* material_specular;
  r32* material_shininess;
  r32* material_ambient;

  int num_skins;
  int* skin_joints;
  r32* skin_inverseBindMatrices;
  r32* skin_bindShapeMatrix;

  int num_animations;
  struct AnimationChannel* animation_channels;
  struct AnimationSampler* animation_samplers;
};
