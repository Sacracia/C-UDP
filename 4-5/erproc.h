#ifndef ERPROC
#define ERPROC

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

typedef struct {
  int type;
  int id;
  char c1, c2;
  int code1, code2;
  int iscoded;
  int pid;
} Package;

int Socket(int domain, int type, int protocol) {
  int res = socket(domain, type, protocol);
  if (res == -1) {
      perror("socket failed");
      exit(-1);
  }
  return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int res = bind(sockfd, addr, addrlen);
  if (res == -1) {
      perror("bind failed");
      exit(-1);
  }
}

#endif