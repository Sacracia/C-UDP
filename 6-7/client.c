#include "erproc.h"

int fd, bfd;

void handler(int sig) {
    if (sig == SIGINT) {
        printf("EXIT!\n");
        close(fd);
        exit(0);
    }
}

struct sockaddr_in broadcastAddr;
void LOG(char* msg) {
    printf("%s", msg);
    sendto(bfd, msg, 5000, 0, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr));
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        printf("Usage: %s <ip> <port> <broadcast ip> <broadcast port>\n", argv[0]);
        exit(0);
    }

    printf("MY PID = %d\n", getpid());
    srand(time(NULL));
    signal(SIGINT, handler);

    const int one = 1;
    fd = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bfd = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      setsockopt(bfd, SOL_SOCKET,SO_BROADCAST, &one, sizeof(one));

    struct sockaddr_in server;
    socklen_t servlen;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port   = htons(atoi(argv[2]));

    // SETUP BROADCAST ADDRESS
      memset(&broadcastAddr, 0, sizeof(broadcastAddr));
      broadcastAddr.sin_family = AF_INET;
      broadcastAddr.sin_addr.s_addr = inet_addr(argv[3]);
      broadcastAddr.sin_port = htons(atoi(argv[4]));

    Package p;
    memset(&p, 0, sizeof p);
    p.id = -1;
    char buf[5000];
    while (p.type != -1) {
        if (p.id != -1) {
            p.code1 = (int)p.c1;
            p.code2 = (int)p.c2;
            p.iscoded = 1;
            p.pid = getpid();
            memset(buf, 0, 5000);
            sprintf(buf, "Clinet %d: coded \'%c\' \'%c\'\n", getpid(), p.c1, p.c2);
            LOG(buf);
        }
        sleep(rand() % 2 + 1);
        sendto(fd, &p, sizeof p, 0, (struct sockaddr *)&server, sizeof(server));
        recvfrom(fd, &p, sizeof p, 0, (struct sockaddr *)&server, &servlen);
    }
    printf("EXIT!\n");
    close(fd);
    exit(0);
}
