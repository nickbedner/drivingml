#pragma once

#include "mana/core/math/ivec3.h"
#include "mana/core/math/mat3.h"
#include "mana/core/math/mat3d.h"
#include "mana/core/math/mat4.h"
#include "mana/core/math/mat4d.h"
#include "mana/core/math/quat.h"
#include "mana/core/math/quatd.h"
#include "mana/core/math/real16.h"
#include "mana/core/math/real32.h"
#include "mana/core/math/real64.h"
#include "mana/core/math/vec2.h"
#include "mana/core/math/vec3.h"
#include "mana/core/math/vec3d.h"
#include "mana/core/math/vec4.h"
#include "mana/core/math/vec4d.h"

r32 degree_to_radian(r32 degree);
r64 degree_to_radian_d(r64 degree);
r32 radian_to_degree(r32 radian);
vec4 vec3_to_vec4(vec3 v1);
vec3 quaternion_to_vec3(quat q1);
mat4 quaternion_to_mat4(quat rotation);
mat4 quaternion_to_mat4_other(quat q);
quat mat4_to_quaternion(mat4 matrix);
vec3 ivec3_to_vec3(ivec3 ivec);
ivec3 vec3_to_ivec3(vec3 vec);
vec3d vec3d_transform(vec3d value, quatd rot);
vec3 vec3d_to_vec3(vec3d vec);
vec3d vec3_to_vec3d(vec3 vec);
mat4 mat4d_to_mat4(mat4d mat);
