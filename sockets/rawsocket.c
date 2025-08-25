#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define PORT 8080

// IP header structure
struct ipheader
{
    unsigned char iph_ihl : 4, iph_ver : 4;
    unsigned char iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned short int iph_flag : 3, iph_offset : 13;
    unsigned char iph_ttl;
    unsigned char iph_protocol;
    unsigned short int iph_chksum;
    struct in_addr iph_sourceip;
    struct in_addr iph_destip;
};

// TCP header structure
struct tcpheader
{
    unsigned short int tcph_srcport;
    unsigned short int tcph_destport;
    unsigned int tcph_seqnum;
    unsigned int tcph_acknum;
    unsigned char tcph_reserved : 4, tcph_offset : 4;
    unsigned char tcph_flags;
    unsigned short int tcph_window;
    unsigned short int tcph_chksum;
    unsigned short int tcph_urgptr;
};

int main()
{

    if (0 == fork())
    {
        // Create web server socket (HTTP port 8080)
        int server_fd, client_fd;
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        char buffer[1024];

        // Create TCP socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
            perror("socket failed");
            exit(1);
        }
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // Bind to port 8080
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(PORT);
        if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind failed");
            exit(1);
        }

        // Listen
        if (listen(server_fd, 5) < 0)
        {
            perror("listen failed");
            exit(1);
        }
        printf("Server listening on http://127.0.0.1:%d\n", PORT);
        while (1)
        {
            // Accept a connection
            client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
            if (client_fd < 0)
            {
                perror("accept failed");
                continue;
            }

            // Read HTTP request
            int n = read(client_fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0';
                printf("Request:\n%s\n", buffer);
            }

            // Send a simple HTTP response
            const char *response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 12\r\n"
                "\r\n"
                "Hello World\n";
            write(client_fd, response, strlen(response));

            close(client_fd);
        }

        close(server_fd);
    }
    else
    {
        int sock;
        char buffer[65536];
        // Create raw socket (capture TCP packets)
        sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
        if (sock < 0)
        {
            perror("Socket error");
            exit(1);
        }

        printf("Sniffer sniffing on port %d... (Ctrl+C to stop)\n", PORT);

        while (1)
        {
            ssize_t data_size = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
            if (data_size < 0)
            {
                perror("Recvfrom error");
                exit(1);
            }

            // Get IP header
            struct ipheader *ip = (struct ipheader *)buffer;
            int ip_header_len = ip->iph_ihl * 4;

            // Get TCP header
            struct tcpheader *tcp = (struct tcpheader *)(buffer + ip_header_len);
            int tcp_header_len = tcp->tcph_offset * 4;

            // Payload start
            char *payload = buffer + ip_header_len + tcp_header_len;
            int payload_len = data_size - (ip_header_len + tcp_header_len);

            if (ntohs(tcp->tcph_destport) == PORT || ntohs(tcp->tcph_srcport) == PORT)
            {
                // Print this packet
                printf("\n=== TCP Segment ===\n");
                printf("From %s:%d -> To %s:%d\n",
                       inet_ntoa(ip->iph_sourceip), ntohs(tcp->tcph_srcport),
                       inet_ntoa(ip->iph_destip), ntohs(tcp->tcph_destport));
                printf("Seq: %u Ack: %u Flags: 0x%x\n",
                       ntohl(tcp->tcph_seqnum), ntohl(tcp->tcph_acknum), tcp->tcph_flags);

                if (payload_len > 0)
                {
                    printf("Payload (%d bytes):\n", payload_len);
                    fwrite(payload, 1, payload_len, stdout);
                    printf("\n");
                }
            }
        }
        close(sock);
    }

    return 0;
}