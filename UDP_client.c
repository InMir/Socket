#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

char buf[] = "Hello there!\n";
int ret;
int main()
{
    int fd = 0;
    struct sockaddr_in addr;
    
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8762); 
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = sendto (fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr));    
    printf("%s\n", buf);
    if(ret < 0)
    {
        perror("send");
        exit(3);
    }
   ret = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
    if(ret < 0)
    {
        perror("recv");
        exit(3);
    }
    printf("%s\n", buf);
    close(fd);
    return 0;
}
