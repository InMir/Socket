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
    int udp = 0, tcp = 0, fd_tcp = 0;
    struct sockaddr_in addr;
    struct pollfd client[2];
    char buf[1024];
    int bytes_read = 0;

    udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(udp < 0)
    {
        perror("socket");
        exit(1);
    }
 
    tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(tcp < 0)
    {
        perror("socket");
        exit(2);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //на одном компе
    
    if(bind(udp, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(3);
    }
 
	addr.sin_port = htons(3426);
 
    if(bind(tcp, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(4);
    }  
	
	listen (tcp, 10);
	
	while (1) {	
		client[0].fd = tcp;
		client[0].events = POLLRDNORM;
		client[1].fd = udp;
		client[1].events = POLLRDNORM;
		poll (client, 2, -1);	
		if (client[0].revents & POLLRDNORM) {
		   	fd_tcp = accept (tcp, NULL, NULL);
		   	if(fd_tcp < 0) {
	       		perror("accept");
	       		exit(5);
		    }
			recv (fd_tcp, buf, sizeof(buf), 0);
			printf ("%s\n", buf);
			close (fd_tcp);
		}
		if (client[1].revents & POLLRDNORM) {
		    bytes_read = recvfrom (udp, buf, sizeof(buf), 0, NULL, NULL);
		    buf[bytes_read] = 0;
			printf ("%s\n", buf);
		}
	}
    close (tcp);
    close (udp);
    return 0;
}
