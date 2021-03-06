// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <float.h>
#include <stdint.h>
#include <stdlib.h>

#include "argdata_impl.h"

argdata_t *argdata_create_float(double value) {
#if DBL_MANT_DIG == 53
  // Extract bits for the floating point value.
  union {
    double f;
    uint64_t i;
  } v = {.f = value};
  uint64_t bits = v.i;
#else
#error "Unknown format for double"
#endif

  // Allocate object with space for floating point value.
  argdata_t *ad = malloc(sizeof(argdata_t) + sizeof(uint64_t) + 1);
  if (ad == NULL)
    return NULL;

  // Store floating point value literally.
  uint8_t *buf = (uint8_t *)(ad + 1);
  *buf = ADT_FLOAT;
  for (size_t i = sizeof(uint64_t); i >= 1; --i) {
    buf[i] = bits;
    bits >>= 8;
  }

  argdata_init_buffer(ad, buf, sizeof(uint64_t) + 1, NULL, NULL);
  return ad;
}
