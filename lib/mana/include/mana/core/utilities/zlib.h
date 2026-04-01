#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mana/core/corecommon.h"

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define ZFAST_BITS 9  // accelerate all cases in default tables
#define ZFAST_MASK ((1 << ZFAST_BITS) - 1)
#define ZNSYMS 288  // number of symbols in literal/length alphabet

typedef struct
{
  u16 fast[1 << ZFAST_BITS];
  u16 firstcode[16];
  int maxcode[17];
  u16 firstsymbol[16];
  u8 size[ZNSYMS];
  u16 value[ZNSYMS];
} zhuffman;

inline global i32 bitreverse16(i32 n) {
  n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
  return n;
}

inline global i32 bit_reverse(i32 v, i32 bits) {
  return bitreverse16(v) >> (16 - bits);
}

global i32 zbuild_huffman(zhuffman* z, const u8* sizelist, i32 num) {
  i32 i, k = 0;
  i32 code, next_code[16], sizes[17];

  // DEFLATE spec for generating codes
  memset(sizes, 0, sizeof(sizes));
  memset(z->fast, 0, sizeof(z->fast));
  for (i = 0; i < num; ++i)
    ++sizes[sizelist[i]];
  sizes[0] = 0;
  for (i = 1; i < 16; ++i)
    if (sizes[i] > (1 << i))
      return 1;  // err("bad sizes", "Corrupt PNG");
  code = 0;
  for (i = 1; i < 16; ++i) {
    next_code[i] = code;
    z->firstcode[i] = (u16)code;
    z->firstsymbol[i] = (u16)k;
    code = (code + sizes[i]);
    if (sizes[i])
      if (code - 1 >= (1 << i))
        return 1;                      // err("bad codelengths", "Corrupt PNG");
    z->maxcode[i] = code << (16 - i);  // preshift for inner loop
    code <<= 1;
    k += sizes[i];
  }
  z->maxcode[16] = 0x10000;  // sentinel
  for (i = 0; i < num; ++i) {
    i32 s = sizelist[i];
    if (s) {
      i32 c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
      u16 fastv = (u16)((s << 9) | i);
      z->size[c] = (u8)s;
      z->value[c] = (u16)i;
      if (s <= ZFAST_BITS) {
        i32 j = bit_reverse(next_code[s], s);
        while (j < (1 << ZFAST_BITS)) {
          z->fast[j] = fastv;
          j += (1 << s);
        }
      }
      ++next_code[s];
    }
  }
  return 1;
}

typedef struct
{
  u8 *zbuffer, *zbuffer_end;
  i32 num_bits;
  u32 code_buffer;

  i8* zout;
  i8* zout_start;
  i8* zout_end;
  i32 z_expandable;

  zhuffman z_length, z_distance;
} zbuf;

inline global i32 zeof(zbuf* z) {
  return (z->zbuffer >= z->zbuffer_end);
}

inline global u8 zget8(zbuf* z) {
  return zeof(z) ? 0 : *z->zbuffer++;
}

global void fill_bits(zbuf* z) {
  do {
    if (z->code_buffer >= (1U << z->num_bits)) {
      z->zbuffer = z->zbuffer_end; /* treat this as EOF so we fail. */
      return;
    }
    z->code_buffer |= (unsigned int)zget8(z) << z->num_bits;
    z->num_bits += 8;
  } while (z->num_bits <= 24);
}

inline global u32 zreceive(zbuf* z, i32 n) {
  u32 k;
  if (z->num_bits < n)
    fill_bits(z);
  k = z->code_buffer & ((1 << n) - 1);
  z->code_buffer >>= n;
  z->num_bits -= n;
  return k;
}

global i32 zhuffman_decode_slowpath(zbuf* a, zhuffman* z) {
  i32 b, s, k;
  // not resolved by fast table, so compute it the slow way
  // use jpeg approach, which requires MSbits at top
  k = bit_reverse((i32)a->code_buffer, 16);
  for (s = ZFAST_BITS + 1;; ++s)
    if (k < z->maxcode[s])
      break;
  if (s >= 16)
    return -1;  // invalid code!
  // code size is s, so:
  b = (k >> (16 - s)) - z->firstcode[s] + z->firstsymbol[s];
  if (b >= ZNSYMS)
    return -1;  // some data was corrupt somewhere!
  if (z->size[b] != s)
    return -1;  // was originally an assert, but report failure instead.
  a->code_buffer >>= s;
  a->num_bits -= s;
  return z->value[b];
}

inline global i32 zhuffman_decode(zbuf* a, zhuffman* z) {
  i32 b, s;
  if (a->num_bits < 16) {
    if (zeof(a)) {
      return -1; /* report error for unexpected end of data. */
    }
    fill_bits(a);
  }
  b = z->fast[a->code_buffer & ZFAST_MASK];
  if (b) {
    s = b >> 9;
    a->code_buffer >>= s;
    a->num_bits -= s;
    return b & 511;
  }
  return zhuffman_decode_slowpath(a, z);
}

global i32 zexpand(zbuf* z, i8* zout, i32 n)  // need to make room for n bytes
{
  i8* q;
  u32 cur, limit, old_limit;
  z->zout = zout;
  if (!z->z_expandable) {
    log_message(LOG_SEVERITY_ERROR, "Output buffer limit: Corrupt PNG");
    return 1;
  }

  cur = (unsigned int)(z->zout - z->zout_start);
  limit = old_limit = (unsigned)(z->zout_end - z->zout_start);
  if (UINT_MAX - cur < (unsigned)n) {
    log_message(LOG_SEVERITY_ERROR, "Out of memory");
    return 1;
  }

  while (cur + (u32)n > limit) {
    if (limit > UINT_MAX / 2) {
      log_message(LOG_SEVERITY_ERROR, "Out of memory");
      return 1;
    }

    limit *= 2;
  }
  q = (i8*)realloc(z->zout_start, limit);
  if (q == NULL) {
    log_message(LOG_SEVERITY_ERROR, "Out of memory");
    return 1;
  }

  z->zout_start = q;
  z->zout = q + cur;
  z->zout_end = q + limit;

  return 1;
}

global const int zlength_base[31] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
global const int zlength_extra[31] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};
global const int zdist_base[32] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};
global const int zdist_extra[32] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

global i32 parse_huffman_block(zbuf* a) {
  i8* zout = a->zout;

  while (TRUE) {
    i32 z = zhuffman_decode(a, &a->z_length);

    if (z < 256) {
      if (z < 0) {
        log_message(LOG_SEVERITY_ERROR, "Bad huffman code: Corrupt PNG");
        return 1;
      }
      if (zout >= a->zout_end) {
        if (!zexpand(a, zout, 1))
          return 0;
        zout = a->zout;
      }

      *zout++ = (char)z;
    } else {
      u8* p;
      i32 len, dist;

      if (z == 256) {
        a->zout = zout;
        return 1;
      }

      z -= 257;
      len = zlength_base[z];

      if (zlength_extra[z])
        len += zreceive(a, zlength_extra[z]);
      z = zhuffman_decode(a, &a->z_distance);

      if (z < 0) {
        log_message(LOG_SEVERITY_ERROR, "Bad huffman code: Corrupt PNG");
        return 2;
      }
      dist = zdist_base[z];

      if (zdist_extra[z])
        dist += zreceive(a, zdist_extra[z]);

      if (zout - a->zout_start < dist) {
        log_message(LOG_SEVERITY_ERROR, "Bad dist: Corrupt PNG");
        return 3;
      }

      if (zout + len > a->zout_end) {
        if (!zexpand(a, zout, len))
          return 0;
        zout = a->zout;
      }

      p = (u8*)(zout - dist);

      if (dist == 1) {  // run of one byte; common in images.
        u8 v = *p;
        if (len) {
          do
            *zout++ = (i8)v;
          while (--len);
        }
      } else {
        if (len) {
          do
            *zout++ = (i8)(*p++);
          while (--len);
        }
      }
    }
  }
}

global int compute_huffman_codes(zbuf* a) {
  global const u8 length_dezigzag[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  zhuffman z_codelength;
  u8 lencodes[286 + 32 + 137];  // padding for maximum single op
  u8 codelength_sizes[19];
  i32 i, n;

  i32 hlit = (i32)zreceive(a, 5) + 257;
  i32 hdist = (i32)zreceive(a, 5) + 1;
  i32 hclen = (i32)zreceive(a, 4) + 4;
  i32 ntot = hlit + hdist;

  memset(codelength_sizes, 0, sizeof(codelength_sizes));
  for (i = 0; i < hclen; ++i) {
    i32 s = (i32)zreceive(a, 3);
    codelength_sizes[length_dezigzag[i]] = (u8)s;
  }
  if (!zbuild_huffman(&z_codelength, codelength_sizes, 19))
    return 0;

  n = 0;
  while (n < ntot) {
    i32 c = zhuffman_decode(a, &z_codelength);
    if (c < 0 || c >= 19)
      return 1;  // err("bad codelengths", "Corrupt PNG");

    if (c < 16)
      lencodes[n++] = (u8)c;
    else {
      u8 fill = 0;
      if (c == 16) {
        c = (i32)zreceive(a, 2) + 3;
        if (n == 0)
          return 1;  // err("bad codelengths", "Corrupt PNG");
        fill = lencodes[n - 1];
      } else if (c == 17)
        c = (i32)zreceive(a, 3) + 3;
      else if (c == 18)
        c = (i32)zreceive(a, 7) + 11;
      else
        return 1;  // err("bad codelengths", "Corrupt PNG");

      if (ntot - n < c)
        return 1;  // err("bad codelengths", "Corrupt PNG");
      memset(lencodes + n, fill, (size_t)c);
      n += c;
    }
  }
  if (n != ntot)
    return 1;  // err("bad codelengths", "Corrupt PNG");
  if (!zbuild_huffman(&a->z_length, lencodes, hlit))
    return 0;
  if (!zbuild_huffman(&a->z_distance, lencodes + hlit, hdist))
    return 0;

  return 1;
}

global int parse_uncompressed_block(zbuf* a) {
  u8 header[4];
  int len, nlen, k;
  if (a->num_bits & 7)
    zreceive(a, a->num_bits & 7);  // discard
  // drain the bit-packed data into header
  k = 0;
  while (a->num_bits > 0) {
    header[k++] = (u8)(a->code_buffer & 255);  // suppress MSVC run-time check
    a->code_buffer >>= 8;
    a->num_bits -= 8;
  }
  if (a->num_bits < 0)
    return 1;  // err("zlib corrupt", "Corrupt PNG");
  // now fill header the normal way
  while (k < 4)
    header[k++] = zget8(a);
  len = header[1] * 256 + header[0];
  nlen = header[3] * 256 + header[2];
  if (nlen != (len ^ 0xffff))
    return 1;  // err("zlib corrupt", "Corrupt PNG");
  if (a->zbuffer + len > a->zbuffer_end)
    return 1;  // err("read past buffer", "Corrupt PNG");
  if (a->zout + len > a->zout_end)
    if (!zexpand(a, a->zout, len))
      return 0;
  memcpy(a->zout, a->zbuffer, (size_t)len);
  a->zbuffer += len;
  a->zout += len;
  return 1;
}

global int parse_zlib_header(zbuf* a) {
  int cmf = zget8(a);
  int cm = cmf & 15;
  /* int cinfo = cmf >> 4; */
  int flg = zget8(a);
  if (zeof(a))
    return 1;  // err("bad zlib header", "Corrupt PNG"); // zlib spec
  if ((cmf * 256 + flg) % 31 != 0)
    return 1;  // err("bad zlib header", "Corrupt PNG"); // zlib spec
  if (flg & 32)
    return 1;  // err("no preset dict", "Corrupt PNG"); // preset dictionary not allowed in png
  if (cm != 8)
    return 1;  // err("bad compression", "Corrupt PNG"); // DEFLATE required for png
  // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
  return 1;
}

global const u8 zdefault_length[ZNSYMS] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                           8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                           8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                           8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                           8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                                           9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                                           9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                                           9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                                           7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8};
global const u8 zdefault_distance[32] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

global int parse_zlib(zbuf* a, i32 parse_header) {
  i32 final, type;
  if (parse_header)
    if (!parse_zlib_header(a))
      return 0;
  a->num_bits = 0;
  a->code_buffer = 0;
  do {
    final = (i32)zreceive(a, 1);
    type = (i32)zreceive(a, 2);
    if (type == 0) {
      if (!parse_uncompressed_block(a))
        return 0;
    } else if (type == 3) {
      return 0;
    } else {
      if (type == 1) {
        // use fixed code lengths
        if (!zbuild_huffman(&a->z_length, zdefault_length, ZNSYMS))
          return 0;
        if (!zbuild_huffman(&a->z_distance, zdefault_distance, 32))
          return 0;
      } else {
        if (!compute_huffman_codes(a))
          return 0;
      }
      if (!parse_huffman_block(a))
        return 0;
    }
  } while (!final);
  return 1;
}

global i32 do_zlib(zbuf* a, i8* obuf, size_t olen, i32 exp, i32 parse_header) {
  a->zout_start = obuf;
  a->zout = obuf;
  a->zout_end = obuf + olen;
  a->z_expandable = exp;

  return parse_zlib(a, parse_header);
}
