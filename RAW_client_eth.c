#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> 
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
#include <net/if.h>

char buf[1024];
int ret;

int main()
{
    int fd = 0;
	struct sockaddr_ll addr;
//    struct sockaddr_in addr;
	struct udphdr udp_str;
	struct iphdr ip_str;
	struct ethhdr eth_str;

    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }
    
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = if_nametoindex("enp0s3");
	addr.sll_halen = 6;

	memset(&udp_str, 0, sizeof(udp_str));
	memset(&ip_str, 0, sizeof(ip_str));
	memset(&eth_str, 0, sizeof(eth_str));

	udp_str.source = htons(54555);
	udp_str.dest = htons(5455);
	udp_str.len = htons(sizeof(buf) - sizeof(ip_str) - sizeof(eth_str));
	udp_str.check = 0;
	
	ip_str.version = 4;
	ip_str.ihl = 5;
	ip_str.tos = 0;
	ip_str.tot_len = htons(sizeof(buf) - sizeof(eth_str));
	ip_str.id = htons(9995);
	ip_str.frag_off = 0;
	ip_str.ttl = 123;
	ip_str.protocol = IPPROTO_UDP;
	ip_str.check = 0;
	ip_str.saddr = inet_addr("192.168.2.102");
	ip_str.daddr = inet_addr("192.168.2.103");
	
	__u16 *ptr = &ip_str;
	__u32 x = 0;
	for(int i = 0; i < 10; i++){
		x += ptr[i];
	}
	x = (x >> 16) + (x&0xFFFF);
	__u16 inv_x = ~x;
	ip_str.check = inv_x;
	
	//printf("-x: %x inv_x: %x", ~x, inv_x);
	
	eth_str.h_dest[0] = 0x08;
	eth_str.h_dest[1] = 0x00;
	eth_str.h_dest[2] = 0x27;
	eth_str.h_dest[3] = 0x90;
	eth_str.h_dest[4] = 0xd3;
	eth_str.h_dest[5] = 0xf1;
	eth_str.h_source[0] = 0x08;
	eth_str.h_source[1] = 0x00;
	eth_str.h_source[2] = 0x27;
	eth_str.h_source[3] = 0x1e;
	eth_str.h_source[4] = 0x09;
	eth_str.h_source[5] = 0xfb;
	eth_str.h_proto = htons(0x0800);
	
	memcpy(buf, &eth_str, sizeof(eth_str));
	memcpy(buf + sizeof(eth_str), &ip_str, sizeof(ip_str));
	memcpy(buf + sizeof(eth_str) + sizeof(ip_str), &udp_str, sizeof(udp_str));
	strcpy(buf + sizeof(eth_str) + sizeof(ip_str) + sizeof(udp_str), "Hello, server");
	
    ret = sendto (fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr));    
    printf("%s\n", buf + 42);
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
	    printf("%s\n", buf + 42);
		if(strcmp(buf + 42,"Hi!") == 0)
			break;	
    }
    close(fd);
    return 0;
}
