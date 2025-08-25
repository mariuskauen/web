#include <arpa/inet.h>
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>

#define LOKAL_PORT 55556
#define BAK_LOGG 10

int main ()
{

   // Deklarasjoner

   struct sockaddr_in  lok_adr;
   int                 sd, ns1, ns2, ns3; // fildeskriptorer for sockets
   int                 len;               // lengde av innlest tekst
   char                buffer[BUFSIZ];    // mellomlager for tekst

   // Oppsett av lyttende socket

   sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
   lok_adr.sin_family      = AF_INET;
   lok_adr.sin_port        = htons((uint16_t)LOKAL_PORT); 
   lok_adr.sin_addr.s_addr = htonl(         INADDR_ANY);
   if ( 0==bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)) )
    fprintf(stderr, "Prosess %d er knyttet til port %d.\nVenter på klient på sd...\n", getpid(), LOKAL_PORT);
   else
    exit(1);
   listen(sd, BAK_LOGG); 

   ns1 = accept(sd, NULL, NULL);    
   printf("Klient forbundet til ns1. Trykk enter for å lukke sd\n");
   read(0, buffer, 1);

   close(sd);

   printf("sd lukket\nTrykk enter for å lese data fra klient\n");
   read(0, buffer, 1);

   // Setter i gang selvstendig lese-/skriveprosess

   len=0;
   if(0==fork()) {
    while (0 < (len = read(ns1, buffer, BUFSIZ) ) )
       write(1, buffer, len);
    exit(1);
   }

   // Lager to ekstra fildeskriptorer
   ns2=dup(ns1);
   ns3=dup(ns1);

   printf("To kopier av fildeskriptor ns1 er opprettet\nTrykk enter for å lukke fildeskriptor ns1.\n");

   read(0, buffer, 1);

   //shutdown(ns1, SHUT_RDWR);
   close(ns1);

   printf("Trykk enter for å lukke fildeskriptor ns2.\n");
   read(0, buffer, 1);
   close(ns2);

   printf("Trykk enter for å lukke fildeskriptor ns3.\n");
   read(0, buffer, 1);
   close(ns3);

   printf("Trykk enter for å avslutte prosessen.\n");
   read(0, buffer, 1);

   return 0;
}