// Copyright (c) 2016-2017 Nuxi (https://nuxi.nl/) and contributors.
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <errno.h>

#include "argdata_impl.h"

void argdata_map_iterate(const argdata_t *ad, argdata_map_iterator_t *it_) {
  struct argdata_map_iterator_impl *it =
      (struct argdata_map_iterator_impl *)it_;
  switch (ad->type) {
    case AD_BUFFER: {
      it->buffer.buffer = ad->buffer.buffer;
      it->buffer.length = ad->length;
      it->buffer.convert_fd = ad->buffer.convert_fd;
      it->buffer.convert_fd_arg = ad->buffer.convert_fd_arg;
      it->error = parse_type(ADT_MAP, &it->buffer.buffer, &it->buffer.length);
      if (it->error == 0) {
        it->type = ADM_BUFFER;
        argdata_map_next(it_);
      } else {
        it->type = ADM_MAP;
        it->map.count = 0;
      }
      break;
    }
    case AD_MAP:
      it->error = 0;
      it->type = ADM_MAP;
      it->map.keys = ad->map.keys;
      it->map.values = ad->map.values;
      it->map.count = ad->map.count;
      break;
    default:
      it->error = EINVAL;
      it->type = ADM_MAP;
      it->map.count = 0;
      break;
  }
}
