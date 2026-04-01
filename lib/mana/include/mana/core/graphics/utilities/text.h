/*#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mana/core/math/advmath.h"

// #include "stb_truetype.h"

#define P(x, y, w, arr) ((vec3){.data[0] = arr[(3 * (((y)*w) + x))], .data[1] = arr[(3 * (((y)*w) + x)) + 1], .data[2] = arr[(3 * (((y)*w) + x)) + 2]})

#define INF -1e24
#define RANGE 1.0
#define EDGE_THRESHOLD 0.02

#ifndef R32_PI
#define R32_PI 3.14159265358979323846
#endif

typedef struct {
  int left_bearing;
  int advance;
  int ix0, ix1;
  int iy0, iy1;
} ex_metrics_t;

internal inline u32 ex_utf8(const char *c) {
  u32 val = 0;

  if ((c[0] & 0xF8) == 0xF0) {
    val |= (c[3] & 0x3F);
    val |= (c[2] & 0x3F) << 6;
    val |= (c[1] & 0x3F) << 12;
    val |= (c[0] & 0x07) << 18;
  } else if ((c[0] & 0xF0) == 0xE0) {
    val |= (c[2] & 0x3F);
    val |= (c[1] & 0x3F) << 6;
    val |= (c[0] & 0x0F) << 12;
  } else if ((c[0] & 0xE0) == 0xC0) {
    val |= (c[1] & 0x3F);
    val |= (c[0] & 0x1F) << 6;
  } else
    val = c[0];

  return val;
}

typedef struct {
  r64 dist;
  r64 d;
} signed_distance_t;

typedef struct {
  int color;
  vec2 p[4];
  int type;
} edge_segment_t;

typedef enum {
  BLACK = 0,
  RED = 1,
  GREEN = 2,
  YELLOW = 3,
  BLUE = 4,
  MAGENTA = 5,
  CYAN = 6,
  WHITE = 7
} edge_color_t;

internal inline r64 median(r64 a, r64 b, r64 c) {
  return MAX(MIN(a, b), MIN(MAX(a, b), c));
}

internal inline int nonzero_sign(r64 n) {
  return 2 * (n > 0) - 1;
}

internal inline int solve_quadratic(r64 x[2], r64 a, r64 b, r64 c) {
  if (fabs(a) < 1e-14) {
    if (fabs(b) < 1e-14) {
      if (c == 0)
        return -1;
      return 0;
    }
    x[0] = -c / b;
    return 1;
  }

  r64 dscr = b * b - 4 * a * c;
  if (dscr > 0) {
    dscr = real64_sqrt(dscr);
    x[0] = (-b + dscr) / (2 * a);
    x[1] = (-b - dscr) / (2 * a);
    return 2;
  } else if (dscr == 0) {
    x[0] = -b / (2 * a);
    return 1;
  } else {
    return 0;
  }
}

internal inline int solve_cubic_normed(r64 *x, r64 a, r64 b, r64 c) {
  r64 a2 = a * a;
  r64 q = (a2 - 3 * b) / 9;
  r64 r = (a * (2 * a2 - 9 * b) + 27 * c) / 54;
  r64 r2 = r * r;
  r64 q3 = q * q * q;
  r64 A, B;
  if (r2 < q3) {
    r64 t = r / real64_sqrt(q3);
    if (t < -1) t = -1;
    if (t > 1) t = 1;
    t = acos(t);
    a /= 3;
    q = -2 * real64_sqrt(q);
    x[0] = q * real64_cos(t / 3) - a;
    x[1] = q * real64_cos((t + 2 * R32_PI) / 3) - a;
    x[2] = q * real64_cos((t - 2 * R32_PI) / 3) - a;
    return 3;
  } else {
    A = -pow(fabs(r) + real64_sqrt(r2 - q3), 1 / 3.);
    if (r < 0) A = -A;
    B = A == 0 ? 0 : q / A;
    a /= 3;
    x[0] = (A + B) - a;
    x[1] = -0.5 * (A + B) - a;
    x[2] = 0.5 * real64_sqrt(3.) * (A - B);
    if (fabs(x[2]) < 1e-14)
      return 2;
    return 1;
  }
}

internal inline int solve_cubic(r64 x[3], r64 a, r64 b, r64 c, r64 d) {
  if (fabs(a) < 1e-14)
    return solve_quadratic(x, b, c, d);

  return solve_cubic_normed(x, b / a, c / a, d / a);
}

internal inline vec2 getortho(vec2 const v, int polarity, int allow_zero) {
  vec2 r;
  r64 len = vec2_len(v);

  if (len == 0) {
    if (polarity) {
      r.data[0] = 0;
      r.data[1] = !allow_zero;
    } else {
      r.data[0] = 0;
      r.data[1] = -!allow_zero;
    }
    return r;
  }

  if (polarity) {
    r.data[0] = -v.data[1] / len;
    r.data[1] = v.data[0] / len;
  } else {
    r.data[0] = v.data[1] / len;
    r.data[1] = -v.data[0] / len;
  }

  return r;
}

internal inline int pixel_clash(const vec3 a, const vec3 b, r64 threshold) {
  int aIn = (a.data[0] > .5f) + (a.data[1] > .5f) + (a.data[2] > .5f) >= 2;
  int bIn = (b.data[0] > .5f) + (b.data[1] > .5f) + (b.data[2] > .5f) >= 2;
  if (aIn != bIn) return 0;
  if ((a.data[0] > .5f && a.data[1] > .5f && a.data[2] > .5f) || (a.data[0] < .5f && a.data[1] < .5f && a.data[2] < .5f) || (b.data[0] > .5f && b.data[1] > .5f && b.data[2] > .5f) || (b.data[0] < .5f && b.data[1] < .5f && b.data[2] < .5f))
    return 0;
  r32 aa, ab, ba, bb, ac, bc;
  if ((a.data[0] > .5f) != (b.data[0] > .5f) && (a.data[0] < .5f) != (b.data[0] < .5f)) {
    aa = a.data[0], ba = b.data[0];
    if ((a.data[1] > .5f) != (b.data[1] > .5f) && (a.data[1] < .5f) != (b.data[1] < .5f)) {
      ab = a.data[1], bb = b.data[1];
      ac = a.data[2], bc = b.data[2];
    } else if ((a.data[2] > .5f) != (b.data[2] > .5f) && (a.data[2] < .5f) != (b.data[2] < .5f)) {
      ab = a.data[2], bb = b.data[2];
      ac = a.data[1], bc = b.data[1];
    } else
      return 0;
  } else if ((a.data[1] > .5f) != (b.data[1] > .5f) && (a.data[1] < .5f) != (b.data[1] < .5f) && (a.data[2] > .5f) != (b.data[2] > .5f) && (a.data[2] < .5f) != (b.data[2] < .5f)) {
    aa = a.data[1], ba = b.data[1];
    ab = a.data[2], bb = b.data[2];
    ac = a.data[0], bc = b.data[0];
  } else
    return 0;

  return (fabsf(aa - ba) >= threshold) && (fabsf(ab - bb) >= threshold) && fabsf(ac - .5f) >= fabsf(bc - .5f);
}

internal inline vec2 linear_direction(edge_segment_t *e, r64 param) {
  return (vec2){.data[0] = e->p[1].data[0] - e->p[0].data[0], .data[1] = e->p[1].data[1] - e->p[0].data[1]};
}

internal inline vec2 quadratic_direction(edge_segment_t *e, r64 param) {
  return vec2_mix(vec2_sub(e->p[1], e->p[0]), vec2_sub(e->p[2], e->p[1]), param);
}

internal inline vec2 cubic_direction(edge_segment_t *e, r64 param) {
  vec2 r;
  vec2 a, b, c, d, t;
  a = vec2_sub(e->p[1], e->p[0]);
  b = vec2_sub(e->p[2], e->p[1]);
  c = vec2_mix(a, b, param);
  a = vec2_sub(e->p[3], e->p[2]);
  d = vec2_mix(b, a, param);
  t = vec2_mix(c, d, param);

  if (!t.data[0] && !t.data[1]) {
    if (param == 0) {
      r.data[0] = e->p[2].data[0] - e->p[0].data[0];
      r.data[1] = e->p[2].data[1] - e->p[0].data[1];
      return r;
    }
    if (param == 1) {
      r.data[0] = e->p[3].data[0] - e->p[1].data[0];
      r.data[1] = e->p[3].data[1] - e->p[1].data[1];
      return r;
    }
  }

  r.data[0] = t.data[0];
  r.data[1] = t.data[1];
  return r;
}

internal inline vec2 direction(edge_segment_t *e, r64 param) {
  switch (e->type) {
    case STBTT_vline: {
      return linear_direction(e, param);
    }
    case STBTT_vcurve: {
      return quadratic_direction(e, param);
    }
    case STBTT_vcubic: {
      return cubic_direction(e, param);
    }
  }
  return VEC2_ZERO;
}

internal inline vec2 linear_point(edge_segment_t *e, r64 param) {
  return vec2_mix(e->p[0], e->p[1], param);
}

internal inline vec2 quadratic_point(edge_segment_t *e, r64 param) {
  vec2 a, b;
  a = vec2_mix(e->p[0], e->p[1], param);
  b = vec2_mix(e->p[1], e->p[2], param);
  return vec2_mix(a, b, param);
}

internal inline vec2 cubic_point(edge_segment_t *e, r64 param) {
  vec2 p12, a, b, c, d;
  p12 = vec2_mix(e->p[1], e->p[2], param);

  a = vec2_mix(e->p[0], e->p[1], param);
  b = vec2_mix(a, p12, param);

  c = vec2_mix(e->p[2], e->p[3], param);
  d = vec2_mix(p12, c, param);

  return vec2_mix(b, d, param);
}

internal inline vec2 point(edge_segment_t *e, r64 param) {
  switch (e->type) {
    case STBTT_vline: {
      return linear_point(e, param);
    }
    case STBTT_vcurve: {
      return quadratic_point(e, param);
    }
    case STBTT_vcubic: {
      return cubic_point(e, param);
    }
  }
  return VEC2_ZERO;
}

// linear edge signed distance
internal inline signed_distance_t linear_dist(edge_segment_t *e, vec2 origin, r64 *param) {
  vec2 aq, ab, eq;
  aq = vec2_sub(origin, e->p[0]);
  ab = vec2_sub(e->p[1], e->p[0]);
  *param = vec2_mul_inner(aq, ab) / vec2_mul_inner(ab, ab);
  eq = vec2_sub(e->p[*param > 0.5], origin);

  r64 endpoint_distance = vec2_len(eq);
  if (*param > 0 && *param < 1) {
    vec2 ab_ortho;
    ab_ortho = getortho(ab, 0, 0);
    r64 ortho_dist = vec2_mul_inner(ab_ortho, aq);
    if (fabs(ortho_dist) < endpoint_distance)
      return (signed_distance_t){ortho_dist, 0};
  }

  ab = vec2_norm(ab);
  eq = vec2_norm(eq);
  r64 dist = nonzero_sign(vec2_cross(aq, ab)) * endpoint_distance;
  r64 d = fabs(vec2_mul_inner(ab, eq));
  return (signed_distance_t){dist, d};
}

// quadratic edge signed distance
internal inline signed_distance_t quadratic_dist(edge_segment_t *e, vec2 origin, r64 *param) {
  vec2 qa, ab, br;
  qa = vec2_sub(e->p[0], origin);
  ab = vec2_sub(e->p[1], e->p[0]);
  br.data[0] = e->p[0].data[0] + e->p[2].data[0] - e->p[1].data[0] - e->p[1].data[0];
  br.data[1] = e->p[0].data[1] + e->p[2].data[1] - e->p[1].data[1] - e->p[1].data[1];

  r64 a = vec2_mul_inner(br, br);
  r64 b = 3 * vec2_mul_inner(ab, br);
  r64 c = 2 * vec2_mul_inner(ab, ab) + vec2_mul_inner(qa, br);
  r64 d = vec2_mul_inner(qa, ab);
  r64 t[3];
  int solutions = solve_cubic(t, a, b, c, d);

  // distance from a
  r64 min_distance = nonzero_sign(vec2_cross(ab, qa)) * vec2_len(qa);
  *param = -vec2_mul_inner(qa, ab) / vec2_mul_inner(ab, ab);
  {
    vec2 a, b;
    a = vec2_sub(e->p[2], e->p[1]);
    b = vec2_sub(e->p[2], origin);

    // distance from b
    r64 distance = nonzero_sign(vec2_cross(a, b)) * vec2_len(b);
    if (fabs(distance) < fabs(min_distance)) {
      min_distance = distance;

      a = vec2_sub(origin, e->p[1]);
      b = vec2_sub(e->p[2], e->p[1]);
      *param = vec2_mul_inner(a, b) / vec2_mul_inner(b, b);
    }
  }

  for (int i = 0; i < solutions; ++i) {
    if (t[i] > 0 && t[i] < 1) {
      // end_point = p[0]+2*t[i]*ab+t[i]*t[i]*br;
      vec2 end_point, a, b;
      end_point.data[0] = e->p[0].data[0] + 2 * t[i] * ab.data[0] + t[i] * t[i] * br.data[0];
      end_point.data[1] = e->p[0].data[1] + 2 * t[i] * ab.data[1] + t[i] * t[i] * br.data[1];

      a = vec2_sub(e->p[2], e->p[0]);
      b = vec2_sub(end_point, origin);
      r64 distance = nonzero_sign(vec2_cross(a, b)) * vec2_len(b);
      if (fabs(distance) <= fabs(min_distance)) {
        min_distance = distance;
        *param = t[i];
      }
    }
  }

  if (*param >= 0 && *param <= 1)
    return (signed_distance_t){min_distance, 0};

  vec2 aa, bb;
  ab = vec2_norm(ab);
  qa = vec2_norm(qa);
  aa = vec2_sub(e->p[2], e->p[1]);
  aa = vec2_norm(aa);
  bb = vec2_sub(e->p[2], origin);
  bb = vec2_norm(bb);

  if (*param < .5)
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(ab, qa))};
  else
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(aa, bb))};
}

// cubic edge signed distance
internal inline signed_distance_t cubic_dist(edge_segment_t *e, vec2 origin, r64 *param) {
  vec2 qa, ab, br, as;
  qa = vec2_sub(e->p[0], origin);
  ab = vec2_sub(e->p[1], e->p[0]);
  br.data[0] = e->p[2].data[0] - e->p[1].data[0] - ab.data[0];
  br.data[1] = e->p[2].data[1] - e->p[1].data[1] - ab.data[1];
  as.data[0] = (e->p[3].data[0] - e->p[2].data[0]) - (e->p[2].data[0] - e->p[1].data[0]) - br.data[0];
  as.data[1] = (e->p[3].data[1] - e->p[2].data[1]) - (e->p[2].data[1] - e->p[1].data[1]) - br.data[1];

  vec2 ep_dir;
  ep_dir = direction(e, 0);

  // distance from a
  r64 min_distance = nonzero_sign(vec2_cross(ep_dir, qa)) * vec2_len(qa);
  *param = -vec2_mul_inner(qa, ep_dir) / vec2_mul_inner(ep_dir, ep_dir);
  {
    vec2 a;
    a = vec2_sub(e->p[3], origin);

    ep_dir = direction(e, 1);
    // distance from b
    r64 distance = nonzero_sign(vec2_cross(ep_dir, a)) * vec2_len(a);
    if (fabs(distance) < fabs(min_distance)) {
      min_distance = distance;

      a.data[0] = origin.data[0] + ep_dir.data[0] - e->p[3].data[0];
      a.data[1] = origin.data[1] + ep_dir.data[1] - e->p[3].data[1];
      *param = vec2_mul_inner(a, ep_dir) / vec2_mul_inner(ep_dir, ep_dir);
    }
  }

  const int search_starts = 4;
  for (int i = 0; i <= search_starts; ++i) {
    r64 t = (r64)i / search_starts;
    for (int step = 0;; ++step) {
      vec2 qpt;
      qpt = point(e, t);
      qpt = vec2_sub(qpt, origin);
      vec2 d;
      d = direction(e, t);
      r64 distance = nonzero_sign(vec2_cross(d, qpt)) * vec2_len(qpt);
      if (fabs(distance) < fabs(min_distance)) {
        min_distance = distance;
        *param = t;
      }
      if (step == search_starts)
        break;

      vec2 d1, d2;
      d1.data[0] = 3 * as.data[0] * t * t + 6 * br.data[0] * t + 3 * ab.data[0];
      d1.data[1] = 3 * as.data[1] * t * t + 6 * br.data[1] * t + 3 * ab.data[1];
      d2.data[0] = 6 * as.data[0] * t + 6 * br.data[0];
      d2.data[1] = 6 * as.data[1] * t + 6 * br.data[1];

      t -= vec2_mul_inner(qpt, d1) / (vec2_mul_inner(d1, d1) + vec2_mul_inner(qpt, d2));
      if (t < 0 || t > 1)
        break;
    }
  }

  if (*param >= 0 && *param <= 1)
    return (signed_distance_t){min_distance, 0};

  vec2 d0, d1;
  d0 = direction(e, 0);
  d1 = direction(e, 1);
  d0 = vec2_norm(d0);
  d1 = vec2_norm(d1);
  qa = vec2_norm(qa);
  vec2 a;
  a = vec2_sub(e->p[3], origin);
  a = vec2_norm(a);

  if (*param < .5)
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(d0, qa))};
  else
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(d1, a))};
}

internal inline void dist_to_pseudo(signed_distance_t *distance, vec2 origin, r64 param, edge_segment_t *e) {
  if (param < 0) {
    vec2 dir, p;
    dir = direction(e, 0);
    dir = vec2_norm(dir);
    vec2 aq = (vec2){.data[0] = origin.data[0], .data[1] = origin.data[1]};
    p = point(e, 0);
    aq = vec2_sub(origin, p);
    r64 ts = vec2_mul_inner(aq, dir);
    if (ts < 0) {
      r64 pseudo_dist = vec2_cross(aq, dir);
      if (fabs(pseudo_dist) <= fabs(distance->dist)) {
        distance->dist = pseudo_dist;
        distance->d = 0;
      }
    }
  } else if (param > 1) {
    vec2 dir, p;
    dir = direction(e, 1);
    dir = vec2_norm(dir);
    vec2 bq = (vec2){.data[0] = origin.data[0], .data[1] = origin.data[1]};
    p = point(e, 1);
    bq = vec2_sub(origin, p);
    r64 ts = vec2_mul_inner(bq, dir);
    if (ts > 0) {
      r64 pseudo_dist = vec2_cross(bq, dir);
      if (fabs(pseudo_dist) <= fabs(distance->dist)) {
        distance->dist = pseudo_dist;
        distance->d = 0;
      }
    }
  }
}

internal inline int signed_compare(signed_distance_t a, signed_distance_t b) {
  return fabs(a.dist) < fabs(b.dist) || (fabs(a.dist) == fabs(b.dist) && a.d < b.d);
}

internal inline int is_corner(vec2 a, vec2 b, r64 threshold) {
  return vec2_mul_inner(a, b) <= 0 || fabs(vec2_cross(a, b)) > threshold;
}

internal inline void switch_color(edge_color_t *color, unsigned long long *seed, edge_color_t banned) {
  edge_color_t combined = *color & banned;
  if (combined == RED || combined == GREEN || combined == BLUE) {
    *color = (edge_color_t)(combined ^ WHITE);
    return;
  }

  if (*color == BLACK || *color == WHITE) {
    internal const edge_color_t start[3] = {CYAN, MAGENTA, YELLOW};
    *color = start[*seed & 3];
    *seed /= 3;
    return;
  }

  int shifted = *color << (1 + (*seed & 1));
  *color = (edge_color_t)((shifted | shifted >> 3) & WHITE);
  *seed >>= 1;
}

internal inline void linear_split(edge_segment_t *e, edge_segment_t *p1, edge_segment_t *p2, edge_segment_t *p3) {
  vec2 p;

  p = point(e, 1 / 3.0);
  memcpy(&p1->p[0], &e->p[0], sizeof(vec2));
  memcpy(&p1->p[1], &p, sizeof(vec2));
  p1->color = e->color;

  p = point(e, 1 / 3.0);
  memcpy(&p2->p[0], &p, sizeof(vec2));
  p = point(e, 2 / 3.0);
  memcpy(&p2->p[1], &p, sizeof(vec2));
  p2->color = e->color;

  p = point(e, 2 / 3.0);
  memcpy(&p3->p[0], &p, sizeof(vec2));
  p = point(e, 2 / 3.0);
  memcpy(&p3->p[1], &e->p[1], sizeof(vec2));
  p3->color = e->color;
}

internal inline void quadratic_split(edge_segment_t *e, edge_segment_t *p1, edge_segment_t *p2, edge_segment_t *p3) {
  vec2 p, a, b;

  memcpy(&p1->p[0], &e->p[0], sizeof(vec2));
  p = vec2_mix(e->p[0], e->p[1], 1 / 3.0);
  memcpy(&p1->p[1], &p, sizeof(vec2));
  p = point(e, 1 / 3.0);
  memcpy(&p1->p[2], &p, sizeof(vec2));
  p1->color = e->color;

  p = point(e, 1 / 3.0);
  memcpy(&p2->p[0], &p, sizeof(vec2));
  a = vec2_mix(e->p[0], e->p[1], 5 / 9.0);
  b = vec2_mix(e->p[1], e->p[2], 4 / 9.0);
  p = vec2_mix(a, b, 0.5);
  memcpy(&p2->p[1], &p, sizeof(vec2));
  p = point(e, 2 / 3.0);
  memcpy(&p2->p[2], &p, sizeof(vec2));
  p2->color = e->color;

  p = point(e, 2 / 3.0);
  memcpy(&p3->p[0], &p, sizeof(vec2));
  p = vec2_mix(e->p[1], e->p[2], 2 / 3.0);
  memcpy(&p3->p[1], &p, sizeof(vec2));
  memcpy(&p3->p[2], &e->p[2], sizeof(vec2));
  p3->color = e->color;
}

internal inline void cubic_split(edge_segment_t *e, edge_segment_t *p1, edge_segment_t *p2, edge_segment_t *p3) {
  vec2 p, a, b, c, d;

  memcpy(&p1->p[0], &e->p[0], sizeof(vec2));  // p1 0
  if (vec2_equals(e->p[0], e->p[1])) {
    memcpy(&p1->p[1], &e->p[0], sizeof(vec2));  // ? p1 1
  } else {
    p = vec2_mix(e->p[0], e->p[1], 1 / 3.0);
    memcpy(&p1->p[1], &p, sizeof(vec2));  // ? p1 1
  }
  a = vec2_mix(e->p[0], e->p[1], 1 / 3.0);
  b = vec2_mix(e->p[1], e->p[2], 1 / 3.0);
  p = vec2_mix(a, b, 1 / 3.0);
  memcpy(&p1->p[2], &p, sizeof(vec2));  // p1 2
  p = point(e, 1 / 3.0);
  memcpy(&p1->p[3], &p, sizeof(vec2));  // p1 3
  p1->color = e->color;

  p = point(e, 1 / 3.0);
  memcpy(&p2->p[0], &p, sizeof(vec2));  // p2 0
  a = vec2_mix(e->p[0], e->p[1], 1 / 3.0);
  b = vec2_mix(e->p[1], e->p[2], 1 / 3.0);
  c = vec2_mix(a, b, 1 / 3.0);
  a = vec2_mix(e->p[1], e->p[2], 1 / 3.0);
  b = vec2_mix(e->p[2], e->p[3], 1 / 3.0);
  d = vec2_mix(a, b, 1 / 3.0);
  p = vec2_mix(c, d, 2 / 3.0);
  memcpy(&p2->p[1], &p, sizeof(vec2));  // p2 1
  a = vec2_mix(e->p[0], e->p[1], 2 / 3.0);
  b = vec2_mix(e->p[1], e->p[2], 2 / 3.0);
  c = vec2_mix(a, b, 2 / 3.0);
  a = vec2_mix(e->p[1], e->p[2], 2 / 3.0);
  b = vec2_mix(e->p[2], e->p[3], 2 / 3.0);
  d = vec2_mix(a, b, 2 / 3.0);
  p = vec2_mix(c, d, 1 / 3.0);
  memcpy(&p2->p[2], &p, sizeof(vec2));  // p2 2
  p = point(e, 2 / 3.0);
  memcpy(&p2->p[3], &p, sizeof(vec2));  // p2 3
  p2->color = e->color;

  p = point(e, 2 / 3.0);
  memcpy(&p3->p[0], &p, sizeof(vec2));  // p3 0

  a = vec2_mix(e->p[1], e->p[2], 2 / 3.0);
  b = vec2_mix(e->p[2], e->p[3], 2 / 3.0);
  p = vec2_mix(a, b, 2 / 3.0);
  memcpy(&p3->p[1], &p, sizeof(vec2));  // p3 1

  if (vec2_equals(e->p[2], e->p[3])) {
    memcpy(&p3->p[2], &e->p[3], sizeof(vec2));  // ? p3 2
  } else {
    p = vec2_mix(e->p[2], e->p[3], 2 / 3.0);
    memcpy(&p3->p[2], &p, sizeof(vec2));  // ? p3 2
  }

  memcpy(&p3->p[3], &e->p[3], sizeof(vec2));  // p3 3
}

internal inline void edge_split(edge_segment_t *e, edge_segment_t *p1, edge_segment_t *p2, edge_segment_t *p3) {
  switch (e->type) {
    case STBTT_vline: {
      linear_split(e, p1, p2, p3);
      break;
    }
    case STBTT_vcurve: {
      quadratic_split(e, p1, p2, p3);
      break;
    }
    case STBTT_vcubic: {
      cubic_split(e, p1, p2, p3);
      break;
    }
  }
}

internal inline r64 shoelace(const vec2 a, const vec2 b) {
  return (b.data[0] - a.data[0]) * (a.data[1] + b.data[1]);
}

internal inline int ex_msdf_glyph_mem(stbtt_fontinfo *font, u32 c, size_t w, size_t h, r32 *bitmap, ex_metrics_t *metrics, int autofit) {
  // Funit to pixel scale
  r32 scale = stbtt_ScaleForMappingEmToPixels(font, h);

  // get glyph bounding box (scaled later)
  int ix0, iy0, ix1, iy1;
  r32 xoff = .5, yoff = .5;
  stbtt_GetGlyphBox(font, stbtt_FindGlyphIndex(font, c), &ix0, &iy0, &ix1, &iy1);

  if (autofit) {
    // calculate new height
    r32 newh = h + (h - (iy1 - iy0) * scale) - 4;

    // calculate new scale
    // see 'stbtt_ScaleForMappingEmToPixels' in stb_truetype.h
    u8 *p = font->data + font->head + 18;
    int unitsPerEm = p[0] * 256 + p[1];
    scale = newh / unitsPerEm;

    // make sure we are centered
    xoff = .0;
    yoff = .0;
  }

  // get left offset and advance
  int left_bearing, advance;
  stbtt_GetGlyphHMetrics(font, stbtt_FindGlyphIndex(font, c), &advance, &left_bearing);
  left_bearing *= scale;

  // calculate offset for centering glyph on bitmap
  int translate_x = (w / 2) - ((ix1 - ix0) * scale) / 2 - left_bearing;
  int translate_y = (h / 2) - ((iy1 - iy0) * scale) / 2 - iy0 * scale;

  // set the glyph metrics
  // (pre-scale them)
  if (metrics) {
    metrics->left_bearing = left_bearing;
    metrics->advance = advance * scale;
    metrics->ix0 = ix0 * scale;
    metrics->ix1 = ix1 * scale;
    metrics->iy0 = iy0 * scale;
    metrics->iy1 = iy1 * scale;
  }

  stbtt_vertex *verts;
  int num_verts = stbtt_GetGlyphShape(font, stbtt_FindGlyphIndex(font, c), &verts);

  // figure out how many contours exist
  int contour_count = 0;
  for (int i = 0; i < num_verts; i++) {
    if (verts[i].type == STBTT_vmove)
      contour_count++;
  }

  if (contour_count == 0) {
    return 0;
  }

  // determin what vertices belong to what contours
  typedef struct {
    size_t start, end;
  } indices_t;
  indices_t *contours = malloc(sizeof(indices_t) * contour_count);
  int j = 0;
  for (int i = 0; i <= num_verts; i++) {
    if (verts[i].type == STBTT_vmove) {
      if (i > 0) {
        contours[j].end = i;
        j++;
      }

      contours[j].start = i;
    } else if (i >= num_verts) {
      contours[j].end = i;
    }
  }

  typedef struct {
    signed_distance_t min_distance;
    edge_segment_t *near_edge;
    r64 near_param;
  } edge_point_t;

  typedef struct {
    edge_segment_t *edges;
    size_t edge_count;
  } contour_t;

  // process verts into series of contour-specific edge lists
  vec2 initial = VEC2_ZERO;  // fix this?
  contour_t *contour_data = malloc(sizeof(contour_t) * contour_count);
  r64 cscale = 64.0;
  for (int i = 0; i < contour_count; i++) {
    size_t count = contours[i].end - contours[i].start;
    contour_data[i].edges = malloc(sizeof(edge_segment_t) * count);
    contour_data[i].edge_count = 0;

    size_t k = 0;
    for (int j = contours[i].start; j < contours[i].end; j++) {
      edge_segment_t *e = &contour_data[i].edges[k];
      stbtt_vertex *v = &verts[j];
      e->type = v->type;
      e->color = WHITE;

      switch (v->type) {
        case STBTT_vmove: {
          vec2 p = (vec2){.data[0] = v->x / cscale, .data[1] = v->y / cscale};
          memcpy(&initial, &p, sizeof(vec2));
          break;
        }

        case STBTT_vline: {
          vec2 p = (vec2){.data[0] = v->x / cscale, .data[1] = v->y / cscale};
          memcpy(&e->p[0], &initial, sizeof(vec2));
          memcpy(&e->p[1], &p, sizeof(vec2));
          memcpy(&initial, &p, sizeof(vec2));
          contour_data[i].edge_count++;
          k++;
          break;
        }

        case STBTT_vcurve: {
          vec2 p = (vec2){.data[0] = v->x / cscale, .data[1] = v->y / cscale};
          vec2 c = (vec2){.data[0] = v->cx / cscale, .data[1] = v->cy / cscale};
          memcpy(&e->p[0], &initial, sizeof(vec2));
          memcpy(&e->p[1], &c, sizeof(vec2));
          memcpy(&e->p[2], &p, sizeof(vec2));

          if ((e->p[0].data[0] == e->p[1].data[0] && e->p[0].data[1] == e->p[1].data[1]) ||
              (e->p[1].data[0] == e->p[2].data[0] && e->p[1].data[1] == e->p[2].data[1])) {
            e->p[1].data[0] = 0.5 * (e->p[0].data[0] + e->p[2].data[0]);
            e->p[1].data[1] = 0.5 * (e->p[0].data[1] + e->p[2].data[1]);
          }

          memcpy(&initial, &p, sizeof(vec2));
          contour_data[i].edge_count++;
          k++;
          break;
        }

        case STBTT_vcubic: {
          vec2 p = (vec2){.data[0] = v->x / cscale, .data[1] = v->y / cscale};
          vec2 c = (vec2){.data[0] = v->cx / cscale, .data[1] = v->cy / cscale};
          vec2 c1 = (vec2){.data[0] = v->cx1 / cscale, .data[1] = v->cy1 / cscale};
          memcpy(&e->p[0], &initial, sizeof(vec2));
          memcpy(&e->p[1], &c, sizeof(vec2));
          memcpy(&e->p[2], &c1, sizeof(vec2));
          memcpy(&e->p[3], &p, sizeof(vec2));
          memcpy(&initial, &p, sizeof(vec2));
          contour_data[i].edge_count++;
          k++;
          break;
        }
      }
    }
  }

  // calculate edge-colors
  unsigned long long seed = 0;
  r64 anglethreshold = 3.0;
  r64 crossthreshold = real64_sin(anglethreshold);
  size_t corner_count = 0;
  for (int i = 0; i < contour_count; ++i)
    for (int j = 0; j < contour_data[i].edge_count; ++j)
      corner_count++;

  int *corners = malloc(sizeof(int) * corner_count);
  int corner_index = 0;
  for (int i = 0; i < contour_count; ++i) {
    if (contour_data[i].edge_count > 0) {
      vec2 prev_dir, dir;
      prev_dir = direction(&contour_data[i].edges[contour_data[i].edge_count - 1], 1);

      int index = 0;
      for (int j = 0; j < contour_data[i].edge_count; ++j, ++index) {
        edge_segment_t *e = &contour_data[i].edges[j];
        dir = direction(e, 0);
        dir = vec2_norm(dir);
        prev_dir = vec2_norm(prev_dir);
        if (is_corner(prev_dir, dir, crossthreshold))
          corners[corner_index++] = index;
        prev_dir = direction(e, 1);
      }
    }

    if (corner_index == 0) {
      for (int j = 0; j < contour_data[i].edge_count; ++j)
        contour_data[i].edges[j].color = WHITE;
    } else if (corner_index == 1) {
      edge_color_t colors[3] = {WHITE, WHITE};
      switch_color(&colors[0], &seed, BLACK);
      colors[2] = colors[0];
      switch_color(&colors[2], &seed, BLACK);

      int corner = corners[0];
      if (contour_data[i].edge_count >= 3) {
        int m = contour_data[i].edge_count;
        for (int j = 0; j < m; ++j)
          contour_data[i].edges[(corner + j) % m].color = (colors + 1)[(int)(3 + 2.875 * i / (m - 1) - 1.4375 + .5) - 3];
      } else if (contour_data[i].edge_count >= 1) {
        edge_segment_t *parts[7] = {NULL};
        edge_split(&contour_data[i].edges[0], parts[0 + 3 * corner], parts[1 + 3 * corner], parts[2 + 3 * corner]);
        if (contour_data[i].edge_count >= 2) {
          edge_split(&contour_data[i].edges[1], parts[3 - 3 * corner], parts[4 - 3 * corner], parts[5 - 3 * corner]);
          parts[0]->color = parts[1]->color = colors[0];
          parts[2]->color = parts[3]->color = colors[1];
          parts[4]->color = parts[5]->color = colors[2];
        } else {
          parts[0]->color = colors[0];
          parts[1]->color = colors[1];
          parts[2]->color = colors[2];
        }
        free(contour_data[i].edges);
        contour_data[i].edges = malloc(sizeof(edge_segment_t) * 7);
        contour_data[i].edge_count = 0;
        int index = 0;
        for (int j = 0; parts[j]; ++j) {
          memcpy(&contour_data[i].edges[index++], &parts[j], sizeof(edge_segment_t));
          contour_data[i].edge_count++;
        }
      }
    } else {
      int spline = 0;
      int start = corners[0];
      int m = contour_data[i].edge_count;
      edge_color_t color = WHITE;
      switch_color(&color, &seed, BLACK);
      edge_color_t initial_color = color;
      for (int j = 0; j < m; ++j) {
        int index = (start + j) % m;
        if (spline + 1 < corner_count && corners[spline + 1] == index) {
          ++spline;

          edge_color_t s = (edge_color_t)((spline == corner_count - 1) * initial_color);
          switch_color(&color, &seed, s);
        }
        contour_data[i].edges[index].color = color;
      }
    }
  }
  free(corners);

  // normalize shape
  for (int i = 0; i < contour_count; i++) {
    if (contour_data[i].edge_count == 1) {
      edge_segment_t *parts[3] = {0};
      edge_split(&contour_data[i].edges[0], parts[0], parts[1], parts[2]);
      free(contour_data[i].edges);
      contour_data[i].edges = malloc(sizeof(edge_segment_t) * 3);
      contour_data[i].edge_count = 3;
      for (int j = 0; j < 3; j++)
        memcpy(&contour_data[i].edges[j], &parts[j], sizeof(edge_segment_t));
    }
  }

  // calculate windings
  int *windings = malloc(sizeof(int) * contour_count);
  for (int i = 0; i < contour_count; i++) {
    size_t edge_count = contour_data[i].edge_count;
    if (edge_count == 0) {
      windings[i] = 0;
      continue;
    }

    r64 total = 0;

    if (edge_count == 1) {
      vec2 a, b, c;
      a = point(&contour_data[i].edges[0], 0);
      b = point(&contour_data[i].edges[0], 1 / 3.0);
      c = point(&contour_data[i].edges[0], 2 / 3.0);
      total += shoelace(a, b);
      total += shoelace(b, c);
      total += shoelace(c, a);
    } else if (edge_count == 2) {
      vec2 a, b, c, d;
      a = point(&contour_data[i].edges[0], 0);
      b = point(&contour_data[i].edges[0], 0.5);
      c = point(&contour_data[i].edges[1], 0);
      d = point(&contour_data[i].edges[1], 0.5);
      total += shoelace(a, b);
      total += shoelace(b, c);
      total += shoelace(c, d);
      total += shoelace(d, a);
    } else {
      vec2 prev;
      prev = point(&contour_data[i].edges[edge_count - 1], 0);
      for (int j = 0; j < edge_count; j++) {
        vec2 cur;
        cur = point(&contour_data[i].edges[j], 0);
        total += shoelace(prev, cur);
        memcpy(&prev, &cur, sizeof(vec2));
      }
    }

    windings[i] = ((0 < total) - (total < 0));
  }

  typedef struct {
    r64 r, g, b;
    r64 med;
  } multi_distance_t;

  multi_distance_t *contour_sd;
  contour_sd = malloc(sizeof(multi_distance_t) * contour_count);

  for (int y = 0; y < h; ++y) {
    int row = iy0 > iy1 ? y : h - y - 1;
    for (int x = 0; x < w; ++x) {
      vec2 p = (vec2){.data[0] = (x + xoff - translate_x) / (scale * 64.0), .data[1] = (y + yoff - translate_y) / (scale * 64.0)};

      edge_point_t sr, sg, sb;
      sr.near_edge = sg.near_edge = sb.near_edge = NULL;
      sr.near_param = sg.near_param = sb.near_param = 0;
      sr.min_distance.dist = sg.min_distance.dist = sb.min_distance.dist = INF;
      sr.min_distance.d = sg.min_distance.d = sb.min_distance.d = 1;
      r64 d = fabs(INF);
      r64 neg_dist = -INF;
      r64 pos_dist = INF;
      int winding = 0;

      for (int j = 0; j < contour_count; ++j) {
        edge_point_t r, g, b;
        r.near_edge = g.near_edge = b.near_edge = NULL;
        r.near_param = g.near_param = b.near_param = 0;
        r.min_distance.dist = g.min_distance.dist = b.min_distance.dist = INF;
        r.min_distance.d = g.min_distance.d = b.min_distance.d = 1;

        for (int k = 0; k < contour_data[j].edge_count; ++k) {
          edge_segment_t *e = &contour_data[j].edges[k];
          r64 param;
          signed_distance_t distance;
          distance.dist = INF;
          distance.d = 1;

          // calculate signed distance
          switch (e->type) {
            case STBTT_vline: {
              distance = linear_dist(e, p, &param);
              break;
            }
            case STBTT_vcurve: {
              distance = quadratic_dist(e, p, &param);
              break;
            }
            case STBTT_vcubic: {
              distance = cubic_dist(e, p, &param);
              break;
            }
          }

          if (e->color & RED && signed_compare(distance, r.min_distance)) {
            r.min_distance = distance;
            r.near_edge = e;
            r.near_param = param;
          }
          if (e->color & GREEN && signed_compare(distance, g.min_distance)) {
            g.min_distance = distance;
            g.near_edge = e;
            g.near_param = param;
          }
          if (e->color & BLUE && signed_compare(distance, b.min_distance)) {
            b.min_distance = distance;
            b.near_edge = e;
            b.near_param = param;
          }
        }

        if (signed_compare(r.min_distance, sr.min_distance))
          sr = r;
        if (signed_compare(g.min_distance, sg.min_distance))
          sg = g;
        if (signed_compare(b.min_distance, sb.min_distance))
          sb = b;

        r64 med_min_dist = fabs(median(r.min_distance.dist, g.min_distance.dist, b.min_distance.dist));

        if (med_min_dist < d) {
          d = med_min_dist;
          winding = -windings[j];
        }

        if (r.near_edge)
          dist_to_pseudo(&r.min_distance, p, r.near_param, r.near_edge);
        if (g.near_edge)
          dist_to_pseudo(&g.min_distance, p, g.near_param, g.near_edge);
        if (b.near_edge)
          dist_to_pseudo(&b.min_distance, p, b.near_param, b.near_edge);

        med_min_dist = median(r.min_distance.dist, g.min_distance.dist, b.min_distance.dist);
        contour_sd[j].r = r.min_distance.dist;
        contour_sd[j].g = g.min_distance.dist;
        contour_sd[j].b = b.min_distance.dist;
        contour_sd[j].med = med_min_dist;

        if (windings[j] > 0 && med_min_dist >= 0 && fabs(med_min_dist) < fabs(pos_dist))
          pos_dist = med_min_dist;
        if (windings[j] < 0 && med_min_dist <= 0 && fabs(med_min_dist) < fabs(neg_dist))
          neg_dist = med_min_dist;
      }

      if (sr.near_edge)
        dist_to_pseudo(&sr.min_distance, p, sr.near_param, sr.near_edge);
      if (sg.near_edge)
        dist_to_pseudo(&sg.min_distance, p, sg.near_param, sg.near_edge);
      if (sb.near_edge)
        dist_to_pseudo(&sb.min_distance, p, sb.near_param, sb.near_edge);

      multi_distance_t msd;
      msd.r = msd.g = msd.b = msd.med = INF;
      if (pos_dist >= 0 && fabs(pos_dist) <= fabs(neg_dist)) {
        msd.med = INF;
        winding = 1;
        for (int i = 0; i < contour_count; ++i)
          if (windings[i] > 0 && contour_sd[i].med > msd.med && fabs(contour_sd[i].med) < fabs(neg_dist))
            msd = contour_sd[i];
      } else if (neg_dist <= 0 && fabs(neg_dist) <= fabs(pos_dist)) {
        msd.med = -INF;
        winding = -1;
        for (int i = 0; i < contour_count; ++i)
          if (windings[i] < 0 && contour_sd[i].med < msd.med && fabs(contour_sd[i].med) < fabs(pos_dist))
            msd = contour_sd[i];
      }

      for (int i = 0; i < contour_count; ++i)
        if (windings[i] != winding && fabs(contour_sd[i].med) < fabs(msd.med))
          msd = contour_sd[i];

      if (median(sr.min_distance.dist, sg.min_distance.dist, sb.min_distance.dist) == msd.med) {
        msd.r = sr.min_distance.dist;
        msd.g = sg.min_distance.dist;
        msd.b = sb.min_distance.dist;
      }

      size_t index = 3 * ((row * w) + x);
      bitmap[index] = (r32)msd.r / RANGE + .5;      // r
      bitmap[index + 1] = (r32)msd.g / RANGE + .5;  // g
      bitmap[index + 2] = (r32)msd.b / RANGE + .5;  // b
    }
  }
  for (int i = 0; i < contour_count; i++)
    free(contour_data[i].edges);
  free(contour_data);
  free(contour_sd);
  free(contours);
  free(windings);
  free(verts);

  // msdf error correction
  typedef struct {
    int x, y;
  } clashes_t;
  clashes_t *clashes = malloc(sizeof(clashes_t) * w * h);
  size_t cindex = 0;

  r64 tx = EDGE_THRESHOLD / (scale * RANGE);
  r64 ty = EDGE_THRESHOLD / (scale * RANGE);
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      if ((x > 0 && pixel_clash(P(x, y, w, bitmap), P(MAX(x - 1, 0), y, w, bitmap), tx)) || (x < w - 1 && pixel_clash(P(x, y, w, bitmap), P(MIN(x + 1, w - 1), y, w, bitmap), tx)) || (y > 0 && pixel_clash(P(x, y, w, bitmap), P(x, MAX(y - 1, 0), w, bitmap), ty)) || (y < h - 1 && pixel_clash(P(x, y, w, bitmap), P(x, MIN(y + 1, h - 1), w, bitmap), ty))) {
        clashes[cindex].x = x;
        clashes[cindex++].y = y;
      }
    }
  }

  for (int i = 0; i < cindex; i++) {
    size_t index = 3 * ((clashes[i].y * w) + clashes[i].x);
    r32 med = median(bitmap[index], bitmap[index + 1], bitmap[index + 2]);
    bitmap[index + 0] = med;
    bitmap[index + 1] = med;
    bitmap[index + 2] = med;
  }
  free(clashes);

  return 1;
}

internal inline r32 *ex_msdf_glyph(stbtt_fontinfo *font, u32 c, size_t w, size_t h, ex_metrics_t *metrics, int autofit) {
  r32 *bitmap = malloc(sizeof(r32) * 3 * w * h);
  memset(bitmap, 0.0f, sizeof(r32) * 3 * w * h);
  if (ex_msdf_glyph_mem(font, c, w, h, bitmap, metrics, autofit) != 1) {
    free(bitmap);
    return NULL;
  }
  return bitmap;
}

#endif  // TEXT_H
*/