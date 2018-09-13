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
#include <netinet/ip.h>

char buf[1024];
int ret;

int main()
{
    int fd = 0;
    struct sockaddr_in addr;
	struct udphdr udp_str;
	struct iphdr ip_str;
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }
    
    u_int yes = 1;
	if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &yes, sizeof(yes)) < 0){
		perror("setsockopt");
		exit(2);
	}
	
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8762); 
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	udp_str.source = htons(23444);
	udp_str.dest = htons(8762);
	udp_str.len = htons(sizeof(buf) - sizeof(ip_str));
	udp_str.check = 0;
	
	ip_str.version = 4;
	ip_str.ihl = 5;
	ip_str.tos = 0;
	ip_str.tot_len = htons(sizeof(buf));
	ip_str.id = htons(9999);
	ip_str.frag_off = 0;
	ip_str.ttl = 123;
	ip_str.protocol = IPPROTO_UDP;
	ip_str.check = 0;
	ip_str.saddr = inet_addr("10.0.2.15");
	ip_str.daddr = inet_addr("127.0.0.1");
	
	
	memcpy(buf, &ip_str, sizeof(ip_str));
	memcpy(buf + sizeof(ip_str), &udp_str, sizeof(udp_str));	
	strcpy(buf + sizeof(ip_str) + sizeof(udp_str), "Hello, server");

    ret = sendto (fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr));    
    printf("%s\n", buf);
    if(ret < 0)
    {
        perror("send");
        exit(3);
    }
    while(1){
	    ret = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
	    if(ret < 0)
	    {
	        perror("recv");
	        exit(4);
	    }
	    printf("%s\n", buf + 28);
		if(strcmp(buf + 28,"New message from client") == 0)
			break;	
    }
    close(fd);
    return 0;
}
