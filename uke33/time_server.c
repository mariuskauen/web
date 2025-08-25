#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>


int socket_listen;

void handle_sigint(int sig) {
    printf("\nGracefully shutting down server...\n");

    printf("Disabling send/recv on listening socket...");
    if(0==shutdown(socket_listen, SHUT_RDWR))
        printf("DONE\n");
    else
    {
        printf("FAILED\n");
    }

    printf("Closing listening socket...");
    if(0==close(socket_listen))
        printf("DONE\n");
    else
        printf("FAILED\n");
        
    printf("Server gracefully shut down.\n");
    exit(0);
}

int main()
{
    signal(SIGINT, handle_sigint);
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    struct addrinfo *bind_address;
    getaddrinfo(0,"8080",&hints,&bind_address);
    
    //printf("%s\n", bind_address->ai_canonname);
    printf("%d\n", bind_address->ai_family);
    printf("%d\n", bind_address->ai_flags);
    printf("%d\n", bind_address->ai_socktype);
    printf("%d\n", bind_address->ai_protocol);

    printf("Creating socket...\n");

    setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,bind_address->ai_protocol);

    if(-1==socket_listen)
    {
        printf("Creating socket failed. %i", socket_listen);
        return 1;
    }
    printf("Binding socket to local address...\n");
    if(0!=bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        printf("Binding socket failed.");
        return 1;
    }
    printf("Listening...\n");
    if(listen(socket_listen, 10) < 0)
    {
        printf("Listening failed.\n");
        return 1;
    }
    while(1)
    {
    printf("Waiting for connection...\n");
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
    if(-1==socket_client)
    {
        printf("Accepting incoming connection failed.\n");
        return 1;
    }
    printf("Client is connected -> ");
    char address_buffer[100];
    getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer),0,0,NI_NUMERICHOST);
    printf("%s\n", address_buffer);

    //printf("Reading request...\n");
    //char request[1024];
    //int bytes_received = recv(socket_client, request, 1024,0);
    //printf("Received %d bytes. \n", bytes_received);
    //printf("%.*s\n", bytes_received, request);
    printf("Sending reponse...\n");
    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response),0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));
    bytes_sent = send(socket_client, time_msg, strlen(time_msg),0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));
    fflush(stdout);

      // Sørger for å stenge socket for skriving og lesing
      // NB! Frigjør ingen plass i fildeskriptortabellen
    shutdown(socket_client, SHUT_RDWR);
    close(socket_client);
    }
    return 0;
}