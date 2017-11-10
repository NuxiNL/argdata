// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stdlib.h>

#include "argdata_writer_impl.h"

void argdata_writer_free(argdata_writer_t *aw) {
  free(aw->control);
  free(aw);
}
