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
#include <netinet/udp.h>

struct pack{
	struct udphdr udp_str;
	char buf[64];
};

char buf[1024];
int ret;
struct pack package;

int main()
{
    int fd = 0;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8762); 
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	package.udp_str.source = htons(23444);
	package.udp_str.dest = htons(8762);
	package.udp_str.len = htons(sizeof(package));
	package.udp_str.check = 0;
	strcpy(package.buf, "Hello, server");

    ret = sendto (fd, &package, sizeof(package), 0, (struct sockaddr*)&addr, sizeof(addr));    
    if(ret < 0)
    {
        perror("send");
        exit(2);
    }
    while(1){
	    ret = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
	    if(ret < 0)
	    {
	        perror("recv");
	        exit(3);
	    }
	    printf("%s\n", buf + 28);
		if(strcmp(buf + 28,"New message from client") == 0)
			break;	
    }
    close(fd);
    return 0;
}
