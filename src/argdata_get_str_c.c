// Copyright (c) 2015 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "argdata.h"

int argdata_get_str_c(const argdata_t *ad, const char **value) {
  // Fetch string value.
  const char *str;
  size_t len;
  int error = argdata_get_str(ad, &str, &len);
  if (error != 0)
    return error;

  // Ensure that it is a C string.
  if (strnlen(str, len + 1) != len)
    return EILSEQ;
  *value = str;
  return 0;
}
