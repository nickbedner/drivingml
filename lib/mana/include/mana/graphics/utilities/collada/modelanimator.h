#pragma once

#include <mana/math/advmath.h>
#include <mana/storage/storage.h>

#include "mana/core/corecommon.h"
#include "mana/graphics/entities/model/modelcommon.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/utilities/xmlparser.h"

void animator_init(struct Animator *animator, struct ModelCommon *entity);
void animator_do_animation(struct Animator *animator, struct Animation *animation);
void animator_update(struct Animator *animator, double delta_time);
void animator_increase_animation_time(struct Animator *animator, double delta_time);
struct Map *animator_calculate_current_animation_pose(struct Animator *animator);
void animator_apply_pose_to_joints(struct Map *current_pose, struct ModelJoint *joint, mat4 parent_transform);
void animator_get_previous_and_next_frames(struct Animator *animator, struct KeyFrame **previous_frame, struct KeyFrame **next_frame);
float animator_calculate_progression(struct Animator *animator, struct KeyFrame *previous_frame, struct KeyFrame *next_frame);
struct Map *animator_interpolate_poses(struct KeyFrame *previous_frame, struct KeyFrame *next_frame, float progression);
