// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stdint.h>

#include "argdata_impl.h"

static const uint8_t buf_false[] = {ADT_BOOL};

const argdata_t argdata_false = {
    .type = AD_BUFFER, .buffer.buffer = buf_false, .length = sizeof(buf_false),
};
