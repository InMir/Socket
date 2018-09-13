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

int main()
{
    int fd = 0;
    struct sockaddr_in addr, addr_save;
    char buf[1024];
    socklen_t size_addr;
    size_addr = sizeof(addr_save);
    int bytes_read = 0;

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8762);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }
	while(1)
    {
        bytes_read = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr_save, &size_addr);
		buf[bytes_read] = 0;
		printf("%s\n", buf);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "New message from client");
		printf("%s\n", buf);
		sendto(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr_save, size_addr);
    }
    close(fd);
    return 0;
}
