#include "erproc.h"

char str[1000];

int bfd;
struct sockaddr_in broadcastAddr;
void LOG(char* msg) {
  printf("%s", msg);
  sendto(bfd, msg, 5000, 0, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr));
}

Package packages[2100];
int parts; // № of packages
char buf[5000];
void PRINT() { // PRINT CURRENT RESULT
    memset(buf, 0, sizeof buf);
    int pos = 0;
    for (int i = 0; i < parts; ++i) {
        if (packages[i].iscoded) {
          if (packages[i].id * 2 + 1 == strlen(str))
            sprintf(buf + strlen(buf), "0x%x ", packages[i].code1);
          else
            sprintf(buf + strlen(buf), "0x%x 0x%x ", packages[i].code1, packages[i].code2);
        }
        else {
          sprintf(buf + strlen(buf), "?? ?? ");
        }
    }
    sprintf(buf + strlen(buf), "\n");
    LOG(buf);
}

int pos = 0;
int id = 0;
void HandleString() { // SPLIT STRING INTO PACKAGES
  parts = (strlen(str) + 1) / 2;
  Package p;
  memset(&p, 0, sizeof(p));
  while (pos < strlen(str)) {
    p.id = id++;
    p.c1 = str[pos++];
    p.c2 = str[pos++];
    p.code1 = -1;
    p.code2 = -1;
    packages[p.id] = p;
  }
}

int curPkg = 0;
Package NotCodedPackage() { // FIND NOT CODED PACKAGE
  for (int i = 0; i < parts; ++i) {
    if (!packages[curPkg].iscoded) {
      Package rtn = packages[curPkg++];
      curPkg %= parts;
      return rtn;
    }
    curPkg = (curPkg + 1) % parts;
  }
  return packages[0];
}

int fd;
void EXIT(int sig) {
  if (sig == SIGINT) {
    LOG("RESULT:\n");
    PRINT();
    close(fd);
    close(bfd);
    exit(0);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    printf("Usage: %s <port> <file> <broadcast IP> <broadcast port>\n", argv[0]);
    exit(0);
  }

  if (atoi(argv[1]) == atoi(argv[4])) {
    printf("Enter port != broadcast port!!!\n");
    exit(0);
  }

  //READ INPUT STRING
  FILE* f = fopen(argv[2], "r");
  if (f == NULL) {
    printf("File not found!\n");
    exit(0);
  }
  char* tmp = fgets(str, 1000, f);
  fclose(f);

  signal(SIGINT, EXIT);
  HandleString();

  const int one = 1;
  fd = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  bfd = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  setsockopt(bfd, SOL_SOCKET,SO_BROADCAST, &one, sizeof(one));

  // SETUP SERVER
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server)); 
  server.sin_family = AF_INET;               
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(atoi(argv[1]));
  Bind(fd, (struct sockaddr *) &server, sizeof(server));

  // SETUP BROADCAST ADDRESS
  memset(&broadcastAddr, 0, sizeof(broadcastAddr));
  broadcastAddr.sin_family = AF_INET;
  broadcastAddr.sin_addr.s_addr = inet_addr(argv[3]);
  broadcastAddr.sin_port = htons(atoi(argv[4]));

  Package p;
  struct sockaddr_in client;
  socklen_t cliAddrLen = sizeof(client);
  int coded = 0; // № of coded packages
  for (;;) {
    recvfrom(fd, &p, sizeof(p), 0, (struct sockaddr *) &client, &cliAddrLen);
    if (p.iscoded && packages[p.id].iscoded == 0) {
      ++coded;
      packages[p.id].iscoded = 1;
      packages[p.id].code1 = p.code1;
      packages[p.id].code2 = p.code2;
      memset(buf, 0, sizeof buf);
      sprintf(buf, "Server: after process %d coded \'%c\' \'%c\':\n", p.pid, p.c1, p.c2);
      LOG(buf);
      PRINT();
    }
    if (coded == parts) {
      printf("All packages are coded!\n");
      EXIT(SIGINT);
    }
    p = NotCodedPackage();
    sendto(fd, &p, sizeof p, 0, (struct sockaddr *) &client, cliAddrLen);
    memset(buf, 0, sizeof buf);
    sprintf(buf, "Server: sending pkg with \'%c\' \'%c\'\n", p.c1, p.c2);
    LOG(buf);
  }
}