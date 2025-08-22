#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#define BUF_SIZE 2048
#define MAX_CLIENTS 100

unsigned char server_mac[6];

struct client_state
{
	uint32_t ip;
	uint16_t port;
	int counter;
};

struct client_state clients[MAX_CLIENTS];
int client_count = 0;
int sockfd;

int find_client(uint32_t ip, uint16_t port)
{
	for (int i = 0; i < client_count; i++)
	{
		if (clients[i].ip == ip && clients[i].port == port)
			return i;
	}
	return -1;
}

void remove_client(uint32_t ip, uint16_t port)
{
	for (int i = 0; i < client_count; i++)
	{
		if (clients[i].ip == ip && clients[i].port == port)
		{
			clients[i] = clients[client_count - 1];
			client_count--;
			return;
		}
	}
}

void sigint_handler(int signo)
{
	close(sockfd);
	printf("\nServer shutdown.\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		fprintf(stderr, "Usage: %s <iface> <server_mac> <server_ip> <server_port>\n", argv[0]);
		exit(1);
	}

	char *iface = argv[1];
	char *server_ip = argv[3];
	int server_port = atoi(argv[4]);


	sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		   &server_mac[0], &server_mac[1], &server_mac[2],
		   &server_mac[3], &server_mac[4], &server_mac[5]);

	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0)
	{
		perror("socket");
		exit(1);
	}

	struct ifreq if_idx, if_mac;
	memset(&if_idx, 0, sizeof(if_idx));
	strncpy(if_idx.ifr_name, iface, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
		perror("SIOCGIFINDEX");

	memset(&if_mac, 0, sizeof(if_mac));
	strncpy(if_mac.ifr_name, iface, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
		perror("SIOCGIFHWADDR");

	struct sockaddr_ll socket_address;
	memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	socket_address.sll_halen = ETH_ALEN;

	signal(SIGINT, sigint_handler);

	unsigned char buffer[BUF_SIZE];

	printf("Server running on %s:%d\n", server_ip, server_port);

	int one = 1;
	if (setsockopt(sockfd, SOL_PACKET, PACKET_IGNORE_OUTGOING, &one, sizeof(one)) < 0)
	{
		perror("setsockopt PACKET_IGNORE_OUTGOING");
	}

	while (1)
	{
		int numbytes = recv(sockfd, buffer, BUF_SIZE, 0);
		if (numbytes < 0)
			continue;

		struct ethhdr *eth = (struct ethhdr *)buffer;
		if (ntohs(eth->h_proto) != ETH_P_IP)
			continue;

		struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
		if (ip->protocol != IPPROTO_UDP)
			continue;
		if (ip->daddr != inet_addr(server_ip))
			continue;

		struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
		char *data = (char *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));
		int data_len = ntohs(udp->len) - sizeof(struct udphdr);
		data[data_len] = '\0';

		if (ntohs(udp->dest) != server_port)
			continue;

		printf("Got from %s:%d -> \"%s\"\n", inet_ntoa(*(struct in_addr *)&ip->saddr), ntohs(udp->source), data);

		if (strcmp(data, "CLOSE") == 0)
		{
			remove_client(ip->saddr, udp->source);
			continue;
		}

		int idx = find_client(ip->saddr, udp->source);
		if (idx == -1)
		{
			idx = client_count++;
			clients[idx].ip = ip->saddr;
			clients[idx].port = udp->source;
			clients[idx].counter = 0;
		}
		clients[idx].counter++;

		char reply[512];
		snprintf(reply, sizeof(reply), "%s %d", data, clients[idx].counter);
		int reply_len = strlen(reply);

		unsigned char sendbuf[BUF_SIZE];
		memset(sendbuf, 0, BUF_SIZE);

		struct ethhdr *eth_out = (struct ethhdr *)sendbuf;
		memcpy(eth_out->h_source, server_mac, ETH_ALEN);
		memcpy(eth_out->h_dest, eth->h_source, ETH_ALEN);
		eth_out->h_proto = htons(ETH_P_IP);

		struct iphdr *ip_out = (struct iphdr *)(sendbuf + sizeof(struct ethhdr));
		ip_out->ihl = 5;
		ip_out->version = 4;
		ip_out->tos = 0;
		ip_out->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + reply_len);
		ip_out->id = htons(0);
		ip_out->frag_off = 0;
		ip_out->ttl = 64;
		ip_out->protocol = IPPROTO_UDP;
		ip_out->saddr = inet_addr(server_ip);
		ip_out->daddr = ip->saddr;
		ip_out->check = 0;

		struct udphdr *udp_out = (struct udphdr *)(sendbuf + sizeof(struct ethhdr) + sizeof(struct iphdr));
		udp_out->source = htons(server_port);
		udp_out->dest = udp->source;
		udp_out->len = htons(sizeof(struct udphdr) + reply_len);
		udp_out->check = 0;

		memcpy(sendbuf + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr), reply, reply_len);

		int pkt_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + reply_len;
		memcpy(socket_address.sll_addr, eth->h_source, ETH_ALEN);

		if (sendto(sockfd, sendbuf, pkt_len, 0, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0)
			perror("sendto");
	}
	return 0;
}
