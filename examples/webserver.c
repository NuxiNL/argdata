// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/socket.h>

#include <argdata.h>
#include <netdb.h>
#include <program.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// A simple webserver that makes use of the Argdata C API to extract its
// parameters.
//
// Usage:
//
//     $ x86_64-unknown-cloudabi-cc -o webserver webserver.c
//     $ cat webserver.yaml
//     %TAG ! tag:nuxi.nl,2015:cloudabi/
//     ---
//     socket: !socket
//         bind: 0.0.0.0:8080
//     logfile: !fd stdout
//     message: <marquee>Hello, world!</marquee>
//     $ cloudabi-run webserver < webserver.yaml

void program_main(const argdata_t *ad) {
  // Default configuration values.
  int sockfd = -1;
  int logfd = -1;
  const char *message = "Hello, world\n";

  // Obtain overrides from configuration.
  {
    argdata_map_iterator_t it;
    argdata_map_iterate(ad, &it);
    const argdata_t *key, *value;
    while (argdata_map_get(&it, &key, &value)) {
      const char *keystr;
      if (argdata_get_str_c(key, &keystr) == 0) {
        if (strcmp(keystr, "socket") == 0)
          argdata_get_fd(value, &sockfd);
        else if (strcmp(keystr, "logfile") == 0)
          argdata_get_fd(value, &logfd);
        else if (strcmp(keystr, "message") == 0)
          argdata_get_str_c(value, &message);
      }
      argdata_map_next(&it);
    }
  }

  // Handle incoming requests.
  {
    int connfd;
    struct sockaddr_storage ss;
    socklen_t sslen = sizeof(ss);
    while ((connfd = accept(sockfd, (struct sockaddr *)&ss, &sslen)) >= 0) {
      // Log request.
      char host[NI_MAXHOST];
      char serv[NI_MAXSERV];
      if (getnameinfo((const struct sockaddr *)&ss, sslen, host, sizeof(host),
                      serv, sizeof(serv),
                      NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
        time_t t = time(NULL);
        char buf[26];
        dprintf(logfd, "[%.24s] Incoming request from %s:%s\n",
                ctime_r(&t, buf), host, serv);
      }

      // Write response to client.
      dprintf(connfd,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n"
              "%s",
              strlen(message), message);
      close(connfd);
    }
  }

  exit(1);
}
