#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <linux/if_packet.h>
#include <net/if.h>

static int sock_fd = -1, bin_dump = -1;
FILE *dump;
unsigned char *buffer = NULL;
struct sockaddr_in source, dest;

void cleanup(int sig)
{
    printf("\rExiting sniffer.\n");
    if (sock_fd != -1)
        close(sock_fd);
    if (bin_dump != -1)
        close(bin_dump);
    if (dump != NULL)
        fclose(dump);
    free(buffer);
    exit(sig == SIGINT ? EXIT_SUCCESS : EXIT_FAILURE);
}

void print_mac(const unsigned char *mac)
{
    fprintf(dump, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void process_packet(unsigned char *buffer, int size)
{
    struct ether_header *ethhdr = (struct ether_header *)buffer;
    struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ether_header));
    struct udphdr *udph = (struct udphdr *)(buffer + sizeof(struct ether_header) + iph->ihl * 4);

    if (iph->protocol != IPPROTO_UDP)
        return;

    fprintf(dump, "=== ETHERNET HEADER ===\n");
    fprintf(dump, "Destination MAC: ");
    print_mac(ethhdr->ether_dhost);
    fprintf(dump, "\n");
    fprintf(dump, "Source MAC: ");
    print_mac(ethhdr->ether_shost);
    fprintf(dump, "\n");
    fprintf(dump, "Type: 0x%04X\n", ntohs(ethhdr->ether_type));

    if (ntohs(ethhdr->ether_type) != ETHERTYPE_IP)
        return;

    fprintf(dump, "\n=== IP HEADER ===\n");
    fprintf(dump, "Version: %d\n", iph->version);
    fprintf(dump, "Header Length: %d bytes\n", iph->ihl * 4);
    fprintf(dump, "TOS: %d\n", iph->tos);
    fprintf(dump, "Total Length: %d\n", ntohs(iph->tot_len));
    fprintf(dump, "ID: %d\n", ntohs(iph->id));
    fprintf(dump, "TTL: %d\n", iph->ttl);
    fprintf(dump, "Protocol: %d\n", iph->protocol);
    fprintf(dump, "Checksum: 0x%X\n", ntohs(iph->check));

    struct in_addr src_addr, dst_addr;
    src_addr.s_addr = iph->saddr;
    dst_addr.s_addr = iph->daddr;
    fprintf(dump, "Source IP: %s\n", inet_ntoa(src_addr));
    fprintf(dump, "Destination IP: %s\n", inet_ntoa(dst_addr));

    fprintf(dump, "\n=== UDP HEADER ===\n");
    fprintf(dump, "Source Port: %d\n", ntohs(udph->source));
    fprintf(dump, "Destination Port: %d\n", ntohs(udph->dest));
    fprintf(dump, "Length: %d\n", ntohs(udph->len));
    fprintf(dump, "Checksum: 0x%X\n", ntohs(udph->check));

    unsigned char *payload = buffer + sizeof(struct ether_header) + iph->ihl * 4 + sizeof(struct udphdr);
    int payload_size = size - (payload - buffer);

    fprintf(dump, "\n=== PAYLOAD (%d bytes) ===\n", payload_size);
    for (int i = 0; i < payload_size; i++)
    {
        fprintf(dump, "%c", payload[i]);
        if ((i + 1) % 16 == 0)
            fprintf(dump, "\n");
    }
    fprintf(dump, "\n\n\n");
}

int main(int argc, char **argv)
{
    signal(SIGINT, cleanup);

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[64], bin_filename[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(filename, sizeof(filename), "dump_%Y-%m-%d_%H-%M-%S.dat", t);
    dump = fopen(filename, "w");
    if (!dump)
    {
        perror("fopen");
        return 1;
    }

    strftime(bin_filename, sizeof(bin_filename), "bin_dump_%Y-%m-%d_%H-%M-%S.dat", t);
    bin_dump = open(bin_filename, O_CREAT | O_WRONLY, 0644);
    if (bin_dump == -1)
    {
        perror("open");
        cleanup(0);
    }

    sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_fd < 0)
    {
        perror("socket");
        cleanup(0);
    }

    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_nametoindex(argv[1]);
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(sock_fd, (struct sockaddr *)&sll, sizeof(sll)) < 0)
    {
        perror("bind");
        cleanup(0);
    }

    buffer = malloc(65536);
    if (!buffer)
    {
        perror("malloc");
        cleanup(0);
    }

    while (1)
    {
        int data_size = recvfrom(sock_fd, buffer, 65536, 0, NULL, NULL);
        if (data_size < 0)
        {
            perror("recvfrom");
            cleanup(0);
        }
        process_packet(buffer, data_size);
    }
}