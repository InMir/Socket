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
    int fd = 0, on = 1;
    struct sockaddr_in addr;
 
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777); 
    addr.sin_addr.s_addr = inet_addr("224.0.0.1");

	struct in_addr inaddr;
	inaddr.s_addr = inet_addr("10.0.2.15");
	
	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &on, sizeof(on)) < 0) {
		perror("setsockopt-mcast_loop");
		exit(2);
	}
	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &inaddr, sizeof(inaddr)) < 0) {
		perror("setsockopt-mcast_if");
		exit(3);
	}
	on = 255;
	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &on, sizeof(on)) < 0) {
		perror("setsockopt-mcast_ttl");
		exit(4);
	}
	

    ret = sendto (fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr));    
    printf("%s\n", buf);
    if(ret < 0)
    {
        perror("send");
        exit(5);
    }
    close(fd);
    return 0;
}
