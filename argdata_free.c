// Copyright (c) 2015 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <stdlib.h>

#include "argdata.h"

void argdata_free(argdata_t *ad) {
  free(ad);
}
