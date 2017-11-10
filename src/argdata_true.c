// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stdint.h>

#include "argdata_impl.h"

static const uint8_t buf_true[] = {ADT_BOOL, 1};

const argdata_t argdata_true = {
    .type = AD_BUFFER, .buffer.buffer = buf_true, .length = sizeof(buf_true),
};
