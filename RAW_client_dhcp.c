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
#include <linux/types.h>

#define DHCPDISCOVER	1
#define DHCPOFFER	2
#define DHCPREQUEST	3
#define DHCPDECLINE	4
#define DHCPACK		5
#define DHCPNAK		6
#define DHCPRELEASE	7
#define DHCPINFORM	8

#define BUF_LEN 1024

char buf[BUF_LEN];
int ret;

uint8_t mac_enp0s3[6] = {0x08, 0x00, 0x27, 0x1e, 0x09, 0xfb}; 
uint8_t mac_enp0s8[6] = {0x08, 0x00, 0x27, 0xbe, 0x70, 0x3c};

__be32 addr_serv = 0;	
__be32 addr_my = 0;	

static const uint8_t magic_cookie[4] = {99, 130, 83, 99};


struct dhcp_hdr{
	struct ethhdr eth_str;
	struct iphdr ip_str;
	struct udphdr udp_str;
	uint8_t op;			/* 1=request, 2=reply */
	uint8_t htype;		/* HW address type */
	uint8_t hlen;		/* HW address length */
	uint8_t hops;		/* Used only by gateways */
	__be32 xid;		/* Transaction ID */
	__be16 secs;		/* Seconds since we started */
	__be16 flags;		/* Just what it says */
	__be32 client_ip;		/* Client's IP address if known */
	__be32 your_ip;		/* Assigned IP address */
	__be32 server_ip;		/* (Next, e.g. NFS) Server's IP address */
	__be32 relay_ip;		/* IP address of BOOTP relay */
	uint8_t hw_addr[16];		/* Client's HW address */
	uint8_t serv_name[64];	/* Server host name */
	uint8_t boot_file[128];	/* Name of boot file */
	uint8_t exten[312];	
	} __attribute__((__packed__));
	
void dhcp_packet (char *buf, char type_addr)
{
	struct dhcp_hdr *dhcph;
	dhcph = (struct dhcp_hdr *)buf;
	memset(buf, 0, BUF_LEN);

	dhcph->udp_str.source = htons(68);
	dhcph->udp_str.dest = htons(67);
	dhcph->udp_str.len = htons(BUF_LEN - sizeof(struct iphdr) - sizeof(struct ethhdr));
	dhcph->udp_str.check = 0;
	
	dhcph->ip_str.version = 4;
	dhcph->ip_str.ihl = 5;
	dhcph->ip_str.tos = 0;
	dhcph->ip_str.tot_len = htons(BUF_LEN - sizeof(struct ethhdr));
	dhcph->ip_str.id = htons(9995);
	dhcph->ip_str.frag_off = 0;
	dhcph->ip_str.ttl = 123;
	dhcph->ip_str.protocol = IPPROTO_UDP;
	dhcph->ip_str.check = 0;
	dhcph->ip_str.daddr = inet_addr("255.255.255.255");
	
	__u16 *ptr = (__u16 *)&(dhcph->ip_str);
	__u32 x = 0;
	for(int i = 0; i < 10; i++){
		x += ptr[i];
	}
	x = (x >> 16) + (x&0xFFFF);
	__u16 inv_x = ~x;
	dhcph->ip_str.check = inv_x;
	
	dhcph->eth_str.h_dest[0] = 0xff;
	dhcph->eth_str.h_dest[1] = 0xff;
	dhcph->eth_str.h_dest[2] = 0xff;
	dhcph->eth_str.h_dest[3] = 0xff;
	dhcph->eth_str.h_dest[4] = 0xff;
	dhcph->eth_str.h_dest[5] = 0xff;
	dhcph->eth_str.h_source[0] = 0x08;
	dhcph->eth_str.h_source[1] = 0x00;
	dhcph->eth_str.h_source[2] = 0x27;
	dhcph->eth_str.h_source[3] = 0x1e;
	dhcph->eth_str.h_source[4] = 0x09;
	dhcph->eth_str.h_source[5] = 0xfb;
	dhcph->eth_str.h_proto = htons(0x0800);
	
	dhcph->op = 1;
	dhcph->htype = 1;
	dhcph->hlen = 6;
	dhcph->hops = 0;
	dhcph->xid = 234;//random number int
	dhcph->secs = 0;
	dhcph->flags = 0;
	dhcph->client_ip = 0;
	dhcph->your_ip = 0;
	dhcph->server_ip = 0;
	dhcph->relay_ip = 0;

	if(type_addr)
		memcpy(dhcph->hw_addr, mac_enp0s3, 6);
	else
		memcpy(dhcph->hw_addr, mac_enp0s8, 6);
		

	uint8_t *e = dhcph->exten; 
	
	uint8_t mt = ((addr_serv == 0)
		 ? DHCPDISCOVER : DHCPREQUEST);

	memcpy(e, magic_cookie, 4);	/* RFC1048 Magic Cookie */
	e += 4;

	*e++ = 53;		/* DHCP message type */
	*e++ = 1;
	*e++ = mt;

	if (mt == DHCPREQUEST) {
		*e++ = 54;	/* Server ID (IP address) */
		*e++ = 4;
		memcpy(e, &addr_serv, 4);
		e += 4;

		*e++ = 50;	/* Requested IP address */
		*e++ = 4;
		memcpy(e, &addr_my, 4);
		e += 4;
	}
	
	*e++ = 255;	/* End of the list */	
}

int main(int argc, char **argv)
{
	if(argc != 2)
		printf("Enter arguments for the programm, 1 - enp0s3 adapter or 0 - enp0s8");
	
	uint8_t *e;
    int fd = 0;
	struct sockaddr_ll addr;
	struct dhcp_hdr *dhcph;
	dhcph = (struct dhcp_hdr *)buf;
	int type_addr = atoi(argv[1]);
	
    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(fd < 0)
    {
        perror("socket");
        exit(1);
    }
    
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = if_nametoindex("enp0s3");
	addr.sll_halen = 6;

	dhcp_packet(buf, type_addr);

    ret = sendto (fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr)); //DHCPDISCOVER sending dhcp discover  
    if(ret < 0)
    {
        perror("DHCPDISCOVER");
        exit(1);
    }
    while(1){
	    ret = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);//DHCPOFFER waitig offer
	    if(ret < 0)
	    {
	        perror("DHCPOFFER");
	        exit(2);
	    }
	    dhcph = (struct dhcp_hdr *)buf;
		e = dhcph->exten; 
	    e += 6;
		if(((memcmp(dhcph->hw_addr, mac_enp0s3, 6) == 0) || (memcmp(dhcph->hw_addr, mac_enp0s8, 6) == 0)) &&
				(*e == DHCPOFFER)) 
			break;
	}
	
	e += 3;
	memcpy(&addr_serv, e, 4);
	addr_my = dhcph->your_ip;
	dhcp_packet(buf, type_addr);
	
	ret = sendto (fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr));//DHCPREQUEST send OK
	if(ret < 0)
	{
		perror("DHCPREQUEST");
		exit(3);
	}
	
	while(1){
		ret = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);//DHCPACK recv ip
		if(ret < 0)
		{
			perror("DHCPACK");
			exit(4);
		}
		dhcph = (struct dhcp_hdr *)buf;
	    e = dhcph->exten;
	    e += 6;
		if(((memcmp(dhcph->hw_addr, mac_enp0s3, 6) == 0) || (memcmp(dhcph->hw_addr, mac_enp0s8, 6) == 0)) &&
				(*e == DHCPACK)) 
		{
			struct in_addr my_ip_addr;
			my_ip_addr.s_addr = addr_my;
			printf("my new ip: %s\n ", inet_ntoa(my_ip_addr));
			break;
		}
    }
    close(fd);
    return 0;
}
