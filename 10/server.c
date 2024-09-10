#include "erproc.h"

int sock;

char str[1000];

Package packages[2100];
int parts; // № of packages
void PRINT() { // PRINT CURRENT RESULT
  for (int i = 0; i < parts; ++i) {
    if (packages[i].iscoded) {
      printf("0x%x 0x%x ", packages[i].code1, packages[i].code2);
    }
    else {
      printf("?? ?? ");
    }
  }
  printf("\n");
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
void* SELF_DESTRUCT(void* args) { // SELF DESTRUCT
  printf("EXIT IN 10 SECONDS!!!!\n");
  for (int i = 0; i < 10; ++i) {
    printf("%d\n", 10 - i);
    sleep(1);
  }
  close(fd);
  printf("RESULT:\n");
  PRINT();
  exit(0);
}

int terminating = 0; // AVOID DOUBLE TERMINATION
void EXIT(int sig) { // ^C HANDLER
  if (sig == SIGINT && terminating == 0) {
    terminating = 1;
    pthread_t tmp;
    pthread_create(&tmp, NULL, SELF_DESTRUCT, NULL);
    struct sockaddr_in client;
    socklen_t cliAddrLen = sizeof(client);
    Package p;
    while (1) {
      recvfrom(fd, &p, sizeof p, 0, (struct sockaddr *)&client, &cliAddrLen);
      p.type = -1;
      sendto(fd, &p, sizeof p, 0, (struct sockaddr *)&client, cliAddrLen);
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s <port> <file>\n", argv[0]);
    exit(0);
  }

  int f = open(argv[2], O_RDONLY);
  if (f < 0) {
    printf("File not found!\n");
	close(f);
    exit(0);
  }
  int read_bytes = read(f, str, sizeof(str));
  close(f);
  
  signal(SIGINT, EXIT);
  HandleString();
  
  fd = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // SETUP SERVER
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server)); 
  server.sin_family = AF_INET;               
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(atoi(argv[1]));
  Bind(fd, (struct sockaddr *) &server, sizeof(server));

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
      printf("After process %d coded \'%c\' \'%c\':\n", p.pid, p.c1, p.c2);
      PRINT();
    }
    if (coded == parts) {
      p.type = -1;
      sendto(fd, &p, sizeof p, 0, (struct sockaddr *)&client, cliAddrLen);
      EXIT(SIGINT);
    }
    p = NotCodedPackage();
    sendto(fd, &p, sizeof p, 0, (struct sockaddr *) &client, cliAddrLen);
  }
  return 0;
}