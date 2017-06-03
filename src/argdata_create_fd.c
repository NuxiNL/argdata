// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "argdata_impl.h"

static int fixed_fd(void *arg, size_t fd) {
  return (intptr_t)arg;
}

argdata_t *argdata_create_fd(int value) {
  // We should only refer to valid file descriptors.
  if (value < 0) {
    errno = EBADF;
    return NULL;
  }

  // Allocate object with space for encoded integer value.
  argdata_t *ad = malloc(sizeof(*ad) + sizeof(uint32_t) + 1);
  if (ad == NULL)
    return NULL;

  // File descriptors are stored as fixed length 32-bit numbers, so that
  // they can be substituted without causing the binary representation
  // to change radically. Just store the value zero for now, as the
  // proper value will be filled in when serializing.
  uint8_t *bufstart = (uint8_t *)(ad + 1), *buf = bufstart;
  *buf++ = ADT_FD;
  encode_fd(0, &buf);

  ad->type = AD_BUFFER;
  ad->buffer.buffer = bufstart;
  ad->length = buf - bufstart;
  ad->buffer.convert_fd = fixed_fd;
  ad->buffer.convert_fd_arg = (void *)(intptr_t)value;
  return ad;
}
