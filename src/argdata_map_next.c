// Copyright (c) 2016-2017 Nuxi (https://nuxi.nl/) and contributors.
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include "argdata_impl.h"

void argdata_map_next(argdata_map_iterator_t *it_) {
  struct argdata_map_iterator_impl *it =
      (struct argdata_map_iterator_impl *)it_;
  if (it->type == ADM_BUFFER) {
    if (it->buffer.length == 0 ||
        (it->error = parse_subfield(&it->buffer.key, &it->buffer.buffer,
                                    &it->buffer.length, it->buffer.convert_fd,
                                    it->buffer.convert_fd_arg)) != 0 ||
        (it->error = parse_subfield(&it->buffer.value, &it->buffer.buffer,
                                    &it->buffer.length, it->buffer.convert_fd,
                                    it->buffer.convert_fd_arg)) != 0) {
      it->type = ADM_MAP;
      it->map.count = 0;
    }
  } else if (it->map.count > 0) {  // type == ADM_MAP
    ++it->map.keys;
    ++it->map.values;
    --it->map.count;
  }
}
