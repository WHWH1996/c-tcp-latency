#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "connection.h"

#define N_BYTES 1000
#define N_ROUNDS 1000000

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    // Init buffers: 1KB
    char rbuffer[N_BYTES] = {[0 ... (N_BYTES - 1)] = 'a'};
    char wbuffer[N_BYTES] = {[0 ... (N_BYTES - 1)] = 'a'};

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }
    printf("Connection successful! Wait a few sec...");
    time_t now, later;
    struct timespec tstart={0,0}, tend={0,0};
    fflush( stdout );

    // SET NON BLOCKING
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    sleep(2);
    int r;
    for (int i = 0; i < N_ROUNDS; i++) {

        clock_gettime(CLOCK_MONOTONIC, &tstart);

        send_message(N_BYTES, sockfd, wbuffer);
        receive_message(N_BYTES, sockfd, rbuffer);

        clock_gettime(CLOCK_MONOTONIC, &tend);

        printf("Round %d took %.9f seconds\n",
            i,
           ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
    }
    close(sockfd);
    return 0;
}

