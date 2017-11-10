// Copyright (c) 2015 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <string.h>

#include "argdata.h"

argdata_t *argdata_create_str_c(const char *value) {
  return argdata_create_str(value, strlen(value));
}
