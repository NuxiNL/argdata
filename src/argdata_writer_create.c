// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stdlib.h>

#include "argdata_writer_impl.h"

argdata_writer_t *argdata_writer_create(void) {
  return calloc(1, sizeof(argdata_writer_t));
}
