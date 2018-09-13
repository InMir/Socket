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
    
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
	mreq.imr_interface.s_addr = inet_addr("10.0.2.15"); 

    char buf[1024] = "Hello!";
    socklen_t size_addr;
    size_addr = sizeof(addr_save);

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }
    
	u_int yes = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0){
		perror("Reusing ADDR failed");
		exit(2);
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) < 0) {
		perror("setsockopt-reuseport");
		exit(3);
	}
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    addr.sin_addr.s_addr = inet_addr("224.0.0.1"); 

    if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(4);
    }  
    
	if(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        perror("multicast");
        close(fd);
        exit(5);
    }
    
	while(1)
    {		
        recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr_save, &size_addr);
		printf("%s\n", buf);
    }
    close(fd);
    return 0;
}
