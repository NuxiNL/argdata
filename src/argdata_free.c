// Copyright (c) 2015 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stdlib.h>

#include "argdata.h"

void argdata_free(argdata_t *ad) {
  free(ad);
}
