// Copyright (c) 2015-2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stdint.h>

#include "argdata_impl.h"

static size_t countfds(const argdata_t *ad) {
  // Skip empty nodes.
  if (ad->length == 0)
    return 0;

  switch (ad->type) {
    case AD_BUFFER: {
      const uint8_t *ibuf = ad->buffer.buffer;
      size_t ilen = ad->length - 1;

      switch (*ibuf++) {
        case ADT_MAP:
        case ADT_SEQ: {
          // Count the number of file descriptors stored in the map or
          // sequence.
          size_t fdslen = 0;
          for (;;) {
            argdata_t iad;
            if (parse_subfield(&iad, &ibuf, &ilen, ad->buffer.convert_fd,
                               ad->buffer.convert_fd_arg) != 0)
              return fdslen;
            fdslen += countfds(&iad);
          }
        }
        case ADT_FD: {
          // A file descriptor.
          uint32_t fd;
          return parse_fd(&fd, &ibuf, &ilen) == 0 ? 1 : 0;
        }
        default:
          // Some other type of object.
          return 0;
      }
    }
    case AD_MAP: {
      // Count the number of file descriptors stored in the map.
      size_t fdslen = 0;
      for (size_t i = 0; i < ad->map.count; ++i) {
        fdslen += countfds(ad->map.keys[i]);
        fdslen += countfds(ad->map.values[i]);
      }
      return fdslen;
    }
    case AD_SEQ: {
      // Count the number of file descriptors stored in the sequence.
      size_t fdslen = 0;
      for (size_t i = 0; i < ad->seq.count; ++i)
        fdslen += countfds(ad->seq.entries[i]);
      return fdslen;
    }
    default:
      // Some other type of object.
      return 0;
  }
}

void argdata_serialized_length(const argdata_t *ad, size_t *datalen,
                               size_t *fdslen) {
  *datalen = ad->length;
  if (fdslen != NULL)
    *fdslen = countfds(ad);
}
