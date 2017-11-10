// Copyright (c) 2017 Nuxi (https://nuxi.nl/) and contributors.
//
// SPDX-License-Identifier: BSD-2-Clause

#include <stdbool.h>

#include "argdata_impl.h"

bool argdata_seq_get(const argdata_seq_iterator_t *it_,
                     const argdata_t **entry) {
  struct argdata_seq_iterator_impl *it =
      (struct argdata_seq_iterator_impl *)it_;
  if (it->type == ADS_BUFFER) {
    *entry = &it->buffer.entry;
    return true;
  } else if (it->seq.count > 0) {  // it->type == ADS_SEQ
    *entry = it->seq.entries[0];
    return true;
  } else {
    return false;
  }
}
