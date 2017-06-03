// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <sys/socket.h>

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include "argdata_reader_impl.h"

// Callback invoked by argdata_get_fd() to translate file descriptor
// numbers in the message to process-local file descriptor numbers.
static int lookup_fd(void *arg, size_t fd) {
  const argdata_reader_t *ar = arg;
  return fd < ar->fds_len ? ar->fds[fd] : -1;
}

int argdata_reader_pull(argdata_reader_t *ar, int fd) {
  // Discard the previously received message.
  if (ar->finished) {
    ar->header_len = 0;
    ar->data_len = 0;
    for (size_t i = 0; i < ar->fds_len; ++i)
      if (ar->fds[i] >= 0)
        close(ar->fds[i]);
    ar->fds_len = 0;
    ar->finished = false;
  }

  // Read the message header.
  while (ar->header_len < sizeof(ar->header)) {
    ssize_t ret = read(fd, ar->header + ar->header_len,
                       sizeof(ar->header) - ar->header_len);
    if (ret == -1) {
      return errno;
    } else if (ret == 0) {
      if (ar->header_len == 0) {
        // Clean end-of-file. No partially read message.
        ar->finished = true;
        return 0;
      }
      return EBADMSG;
    }
    ar->header_len += ret;
  }

  // Extract the message length and number of descriptors from the header.
  uint64_t data_len =
      (uint64_t)ar->header[0] << 56 | (uint64_t)ar->header[1] << 48 |
      (uint64_t)ar->header[2] << 40 | (uint64_t)ar->header[3] << 32 |
      (uint64_t)ar->header[4] << 24 | (uint64_t)ar->header[5] << 16 |
      (uint64_t)ar->header[6] << 8 | (uint64_t)ar->header[7];
  uint32_t fds_len = (uint32_t)ar->header[8] << 24 |
                     (uint32_t)ar->header[9] << 16 |
                     (uint32_t)ar->header[10] << 8 | (uint32_t)ar->header[11];
  if (data_len > ar->max_data_len || fds_len > ar->max_fds_len)
    return EMSGSIZE;

  // Perform the first read through recvmsg() in case the message has
  // file descriptors attached to it.
  char *data = ar->control + CMSG_SPACE(ar->max_fds_len * sizeof(int));
  if (fds_len > 0 && ar->fds_len == 0) {
    struct iovec iov = {
        .iov_base = data + ar->data_len, .iov_len = data_len - ar->data_len,
    };
    struct msghdr msg = {
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = ar->control,
        .msg_controllen = CMSG_SPACE(fds_len * sizeof(int)),
    };
    ssize_t ret = recvmsg(fd, &msg, 0);
    if (ret == -1)
      return errno;

    // Extract the file descriptor table immediately, so that its file
    // descriptors are properly garbage collected.
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
         cmsg = CMSG_NXTHDR(&msg, cmsg)) {
      if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        ar->fds = (int *)CMSG_DATA(cmsg);
        ar->fds_len = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
        break;
      }
    }
    if (ret == 0 || (msg.msg_flags & MSG_CTRUNC) != 0)
      return EBADMSG;
    ar->data_len += ret;
  }
  if (fds_len != ar->fds_len)
    return EBADMSG;

  // Read remaining data through read().
  while (ar->data_len < data_len) {
    ssize_t ret = read(fd, data + ar->data_len, data_len - ar->data_len);
    if (ret == -1) {
      return errno;
    } else if (ret == 0) {
      return EBADMSG;
    }
    ar->data_len += ret;
  }

  // Read completed.
  argdata_init_buffer(&ar->root, data, ar->data_len, lookup_fd, ar);
  ar->finished = true;
  return 0;
}
