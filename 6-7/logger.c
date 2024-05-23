#include "erproc.h"

const int one = 1;

int fd;
void handler(int sig) {
  if (sig == SIGINT) {
    close(fd);
    exit(0);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <broadcast port>\n", argv[0]);
    exit(0);
  }

  signal(SIGINT, handler);

  fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  setsockopt(fd, SOL_SOCKET, SO_BROADCAST,&one, sizeof(one));

  struct sockaddr_in broadcastAddr;
  memset(&broadcastAddr, 0, sizeof(broadcastAddr));
  broadcastAddr.sin_family = AF_INET; 
  broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  broadcastAddr.sin_port = htons(atoi(argv[1]));
  bind(fd, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr));

  char buf[5000];
  for (;;) {
    recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
    printf("%s", buf);
  }
}