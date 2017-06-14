// Copyright (c) 2016-2017 Nuxi (https://nuxi.nl/) and contributors.
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include "argdata_impl.h"

void argdata_seq_next(argdata_seq_iterator_t *it_) {
  struct argdata_seq_iterator_impl *it =
      (struct argdata_seq_iterator_impl *)it_;
  if (it->type == ADS_BUFFER) {
    if (it->buffer.length == 0 ||
        (it->error = parse_subfield(&it->buffer.entry, &it->buffer.buffer,
                                    &it->buffer.length, it->buffer.convert_fd,
                                    it->buffer.convert_fd_arg)) != 0) {
      it->type = ADS_SEQ;
      it->seq.count = 0;
    }
  } else if (it->seq.count > 0) {  // type == ADS_SEQ
    ++it->seq.entries;
    --it->seq.count;
  }
}
