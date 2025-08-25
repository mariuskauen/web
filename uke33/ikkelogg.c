#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define LOKAL_PORT 55556
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler

int main()
{

    struct sockaddr_in lok_adr;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int sd, ny_sd;

    // Setter opp socket-strukturen
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // For at operativsystemet ikke skal holde porten reservert etter tjenerens død
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    // Initierer lokal adresse
    lok_adr.sin_family = AF_INET;
    lok_adr.sin_port = htons((uint16_t)LOKAL_PORT);
    lok_adr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Kobler sammen socket og lokal adresse
    if (0 == bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)))
        fprintf(stderr, "Prosess %d er knyttet til port %d.\n", getpid(), LOKAL_PORT);
    else
        exit(1);

    // Venter på forespørsel om forbindelse
    listen(sd, BAK_LOGG);
    while (1)
    {

        // Aksepterer mottatt forespørsel
        ny_sd = accept(sd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (0 == fork())
        {
            if(0!=chroot("/var/hallo/"))
                exit(1);
            int logfd = open("/var/log/hallo/log", O_CREAT | O_RDWR, O_APPEND);
            
            dup2(ny_sd, 1); // redirigerer socket til standard utgang
            char client_ip_str[INET_ADDRSTRLEN];

            struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
            inet_ntop(AF_INET, &(s->sin_addr), client_ip_str, sizeof(client_ip_str));
            write(logfd, client_ip_str, strlen(client_ip_str));
            write(logfd, "\n", 1);

            printf("HTTP/1.1 200 OK\n");
            printf("Content-Type: text/plain\n");
            printf("\n");
            printf("Your IP has been logged!\n");

            fflush(stdout);
            close(logfd);
            // Sørger for å stenge socket for skriving og lesing
            // NB! Frigjør ingen plass i fildeskriptortabellen
            shutdown(ny_sd, SHUT_RDWR);
            exit(0);
        }

        else
        {
            close(ny_sd);
        }
    }
    return 0;
}