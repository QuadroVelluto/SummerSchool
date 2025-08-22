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

int sockfd;
char *iface, *client_ip, *server_ip;
int client_port, server_port;
unsigned char server_mac[6];
unsigned char client_mac[6];

void send_msg(const char *msg)
{
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
	memcpy(socket_address.sll_addr, server_mac, 6);

	unsigned char sendbuf[BUF_SIZE];
	memset(sendbuf, 0, BUF_SIZE);

	struct ethhdr *eth = (struct ethhdr *)sendbuf;
	memcpy(eth->h_source, client_mac, ETH_ALEN);
	memcpy(eth->h_dest, server_mac, ETH_ALEN);
	eth->h_proto = htons(ETH_P_IP);

	struct iphdr *ip = (struct iphdr *)(sendbuf + sizeof(struct ethhdr));
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	int msg_len = strlen(msg);
	ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + msg_len);
	ip->id = htons(0);
	ip->frag_off = 0;
	ip->ttl = 64;
	ip->protocol = IPPROTO_UDP;
	ip->saddr = inet_addr(client_ip);
	ip->daddr = inet_addr(server_ip);
	ip->check = 0;

	struct udphdr *udp = (struct udphdr *)(sendbuf + sizeof(struct ethhdr) + sizeof(struct iphdr));
	udp->source = htons(client_port);
	udp->dest = htons(server_port);
	udp->len = htons(sizeof(struct udphdr) + msg_len);
	udp->check = 0;

	memcpy(sendbuf + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr), msg, msg_len);

	int pkt_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + msg_len;

	if (sendto(sockfd, sendbuf, pkt_len, 0, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0)
		perror("sendto");
}

void sigint_handler(int signo)
{
	send_msg("CLOSE");
	close(sockfd);
	printf("\nClient closed.\n");
	exit(0);
}

void recv_loop()
{
	unsigned char buf[BUF_SIZE];
	while (1)
	{
		int n = recv(sockfd, buf, BUF_SIZE, 0);
		if (n < 0)
			continue;

		struct ethhdr *eth = (struct ethhdr *)buf;

		if (ntohs(eth->h_proto) != ETH_P_IP)
			continue;

		if (memcmp(eth->h_source, server_mac, 6) != 0)
			continue;

		struct iphdr *ip = (struct iphdr *)(buf + sizeof(struct ethhdr));

		if (ip->saddr != inet_addr(server_ip))
			continue;

		if (ip->protocol != IPPROTO_UDP)
			continue;

		struct udphdr *udp = (struct udphdr *)(buf + sizeof(struct ethhdr) + sizeof(struct iphdr));

		if (ntohs(udp->dest) != client_port)
			continue;

		int data_len = ntohs(udp->len) - sizeof(struct udphdr);
		char *data = (char *)(buf + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));
		data[data_len] = '\0';

		printf("\nReply from server: \"%s\"\n", data);
		fflush(stdout);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 8)
	{
		fprintf(stderr, "Usage: %s <iface> <client_mac> <client_ip> <client_port> <server_mac> <server_ip> <server_port>\n", argv[0]);
		exit(1);
	}

	iface = argv[1];
	client_ip = argv[3];
	client_port = atoi(argv[4]);
	server_ip = argv[6];
	server_port = atoi(argv[7]);

	sscanf(argv[5], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		   &server_mac[0], &server_mac[1], &server_mac[2],
		   &server_mac[3], &server_mac[4], &server_mac[5]);
		   
	sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		   &client_mac[0], &client_mac[1], &client_mac[2],
		   &client_mac[3], &client_mac[4], &client_mac[5]);

	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0)
	{
		perror("socket");
		exit(1);
	}

	pid_t pid = fork();
	if (pid == 0)
	{
		int one = 1;
		if (setsockopt(sockfd, SOL_PACKET, PACKET_IGNORE_OUTGOING, &one, sizeof(one)) < 0)
			perror("setsockopt PACKET_IGNORE_OUTGOING");

		signal(SIGINT, sigint_handler);
		recv_loop();
		exit(0);
	}

	char msg[256];
	while (1)
	{
		printf("Enter message: ");
		if (!fgets(msg, sizeof(msg), stdin))
			break;
		msg[strcspn(msg, "\n")] = 0;
		send_msg(msg);
	}

	return 0;
}
