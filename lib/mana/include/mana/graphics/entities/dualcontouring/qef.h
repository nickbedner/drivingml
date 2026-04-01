#pragma once

#include <string.h>

#include "mana/math/advmath.h"

struct QefData {
  r32 ata_00, ata_01, ata_02, ata_11, ata_12, ata_22;
  r32 atb_x, atb_y, atb_z;
  r32 btb;
  r32 mass_point_x, mass_point_y, mass_point_z;
  i32 num_points;
};

void qef_data_init(struct QefData* qef_data);
void qef_data_init_copy(struct QefData* qef_data, struct QefData* rhs);
void qef_data_add(struct QefData* qef_data, struct QefData* rhs);
void qef_data_clear(struct QefData* qef_data);
void qef_data_set(struct QefData* qef_data, const r32 ata_00, const r32 ata_01, const r32 ata_02, const r32 ata_11, const r32 ata_12, const r32 ata_22, const r32 atb_x, const r32 atb_y, const r32 atb_z, const r32 btb, const r32 mass_point_x, const r32 mass_point_y, const r32 mass_point_z, const int num_points);
void qef_data_set_copy(struct QefData* qef_data, struct QefData* rhs);

struct QefSolver {
  struct QefData data;
  mat3 ata;
  vec3 atb;
  vec3 mass_point;
  vec3 x;
  r32 last_error;
  b8 has_solution;
};

void qef_solver_init(struct QefSolver* qef_solver);
void qef_solver_add(struct QefSolver* qef_solver, const r32 px, const r32 py, const r32 pz, r32 nx, r32 ny, r32 nz);
void qef_solver_add_simp(struct QefSolver* qef_solver, vec3 p, vec3 n);
void qef_solver_add_vec3(struct QefSolver* qef_solver, const vec3 p, const vec3 n);
void qef_solver_add_copy(struct QefSolver* qef_solver, struct QefData* rhs);
r32 qef_solver_get_error(struct QefSolver* qef_solver);
r32 qef_solver_get_error_pos(struct QefSolver* qef_solver, vec3 pos);
void qef_solver_reset(struct QefSolver* qef_solver);
void qef_solver_set_ata(struct QefSolver* qef_solver);
void qef_solver_set_atb(struct QefSolver* qef_solver);
r32 qef_solver_solve(struct QefSolver* qef_solver, vec3* outx, const r32 svd_tol, const int svd_sweeps, const r32 pinv_tol);
