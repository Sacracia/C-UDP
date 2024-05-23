#include "erproc.h"

int fd;
int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(0);
    }
    
    printf("MY PID = %d\n", getpid());
    srand(time(NULL));
    fd = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    struct sockaddr_in server;
    socklen_t servlen;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port   = htons(atoi(argv[2]));

    Package p;
    memset(&p, 0, sizeof p);
    p.id = -1;
    while (p.type != -1) {
        if (p.id != -1) {
            p.code1 = (int)p.c1;
            p.code2 = (int)p.c2;
            p.iscoded = 1;
            p.pid = getpid();
            printf("Coded \'%c\' \'%c\'\n", p.c1, p.c2);
        }
        sleep(rand() % 2 + 1);
        sendto(fd, &p, sizeof p, 0, (struct sockaddr *)&server, sizeof(server));
        recvfrom(fd, &p, sizeof p, 0, (struct sockaddr *)&server, &servlen);
    }
    printf("EXIT!\n");
    close(fd);
    exit(0);
}
