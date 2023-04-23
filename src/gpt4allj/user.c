#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#include "user.h"

static int sock = 0;
static struct sockaddr_storage peer_addr;
static socklen_t peer_addrlen;
static int peer_set = 0;

int user_init(uint16_t port) {
  struct sockaddr_storage addr;
  struct sockaddr_in *inaddr;
  socklen_t addrlen;
  int r = -1;

  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1) {
    memset(&addr, 0, sizeof(struct sockaddr_storage));
    inaddr = (struct sockaddr_in *)&addr;
    inaddr->sin_family = AF_INET;
    inaddr->sin_addr.s_addr = inet_addr("0.0.0.0");
    inaddr->sin_port = htons(port);
    addrlen = sizeof(struct sockaddr_in);

    if ((r = bind(sock, (struct sockaddr *)&addr, addrlen)) != -1) {
      listen(sock, 5);
      fprintf(stderr, "listening on port %d\n", port);
      r = 0;
    } else {
      fprintf(stderr, "bind() failed\n");
    }
  } else {
    fprintf(stderr, "socket() failed\n");
  }

  return r;
}

void user_finish(void) {
  if (sock > 0) close(sock);
  sock = 0;
}

int user_input(char *buf, uint16_t max) {
  struct sockaddr_in *inaddr;
  struct sockaddr_in6 *in6addr;
  uint16_t port;
  char *h, host[256];
  int r = -1;

  if (sock > 0) {
    peer_addrlen = sizeof(struct sockaddr);
    r = recvfrom(sock, buf, max, 0, (struct sockaddr *)&peer_addr, &peer_addrlen);
    if (r > 0) {
      buf[r] = 0;

      switch (peer_addr.ss_family) {
        case AF_INET:
          inaddr = (struct sockaddr_in *)&peer_addr;
          h = inet_ntoa(inaddr->sin_addr);
          port = ntohs(inaddr->sin_port);
          peer_set = 1;
          break;
        case AF_INET6:
          in6addr = (struct sockaddr_in6 *)&peer_addr;
          inet_ntop(AF_INET6, &(in6addr->sin6_addr), host, sizeof(host)-1);
          port = ntohs(in6addr->sin6_port);
          fprintf(stderr, "received \"%.*s\" from inet6 %s:%d (ignored)\n", r, buf, host, port);
          r = -1;
          break;
        default:
          fprintf(stderr, "received invalid address family %d\n", peer_addr.ss_family);
          r = -1;
          break;
      }
    } else {
      fprintf(stderr, "recvfrom() failed (%d)\n", r);
    }
  }

  return r;
}

int user_output(const char *buf) {
  struct sockaddr_in *inaddr;
  struct timeval timeout = { 0, 0 };
  fd_set fds;
  char *s;

  if (sock > 0 && peer_set) {
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    if (select(sock+1, &fds, NULL, NULL, &timeout)) return 0;

    inaddr = (struct sockaddr_in *)&peer_addr;
    s = (char *)inet_ntoa(inaddr->sin_addr);
    if (sendto(sock, buf, strlen(buf)+1, 0, (struct sockaddr *)&peer_addr, peer_addrlen) == -1) {
      fprintf(stderr, "sendto() failed\n");
    }
    return 1;
  }

  return 0;
}
