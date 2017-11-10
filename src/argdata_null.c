// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stddef.h>

#include "argdata_impl.h"

const argdata_t argdata_null = {
    .type = AD_BUFFER, .buffer.buffer = NULL, .length = 0,
};
