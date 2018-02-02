/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>  /* signal name macros, and the kill() prototype */
#include <ctype.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

// We want to parse the buffer and save the content type 
void getContentType(char *buffer, char *contentType, char *filename, char *filetype) 
{
    // printf("hello buffer %s\n", buffer);

    int i = 0;
    int lastSpaceIdx = -1;
    int filetypeIdx = -1;
    int filenameIdx = -1;
    // char filetype[56];

    while (i < strlen(buffer) && buffer[i] != '\n') {
        i++;
    }
    while (i > 0 && buffer[i] != ' ') {
        i--;
    }
    lastSpaceIdx = i;
    // printf("lastSpaceIdx %d\n", lastSpaceIdx);
    while (i > 0 && buffer[i] != '.') {
        i--;
    }
    filetypeIdx = i + 1;
    // printf("filetypeIdx %d\n", filetypeIdx);

    while (i > 0 && buffer[i] != '/') {
        i--;
    }
    // printf("I am at char %c\n", buffer[i]);
    filenameIdx = i + 1;

    // printf("filetypeIdx: %d, lastSpaceIdx %d\n", filetypeIdx, lastSpaceIdx);
    // for (int j = filetypeIdx; j < lastSpaceIdx; j++) {
        // printf("%c\n", buffer[j]);
    // }
    memcpy(filetype, buffer + filetypeIdx, (lastSpaceIdx - filetypeIdx));
    memcpy(filename, buffer + filenameIdx, (lastSpaceIdx - filenameIdx));
    // printf("Filetype is %s\n", filetype);

    for (int k = 0; k < strlen(filetype); k++) {
        filetype[k] = tolower(filetype[k]);
    }
    // printf("Lowered filetype is %s\n", filetype);
    if (strcmp(filetype, "html") == 0 || strcmp(filetype, "htm") == 0) {
        printf("I am an %s file!\n", filetype);
        sprintf(contentType, "text/html");
    }
    else if (strcmp(filetype, "jpg") == 0 || strcmp(filetype, "jpeg") == 0) {
        printf("I am a %s file!\n", filetype);
        sprintf(contentType, "image/jpeg");
    }
    else if (strcmp(filetype, "gif") == 0) {
        printf("I am a %s file!\n", filetype);
        sprintf(contentType, "image/gif");
    }
    else {
        printf("I don't support your filetype!\n");
        sprintf(contentType, "application/octet-stream");
    }
}


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // create socket
    if (sockfd < 0)
        error("ERROR opening socket");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));   // reset memory

    // fill in address info
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);  // 5 simultaneous connection at most

    //accept connections
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0)
     error("ERROR on accept");

    int n;
    int ret;
    char filename[256];
    char contentType[128];
    char buffer[1024];
    char method[128];
    char fileExt[56];

    memset(filename, 0, 256);  // reset memory
    memset(contentType, 0, 128);  // reset memory
    memset(buffer, 0, 1024);  // reset memory
    memset(method, 0, 128);  // reset memory
    memset(fileExt, 0, 56);  // reset memory
    

    //read client's message
    n = read(newsockfd, buffer, 1024);
    if (n < 0) error("ERROR reading from socket");
    printf("%s", buffer);

    memcpy(method, buffer, 4);
    ret = strcmp(method, "GET ");
    if (ret == 0) {
        // printf("This is a GET request.\n");
        getContentType(buffer, contentType, filename, fileExt);
        printf("buffer is %s, contentType is %s, filename is %s, fileExt is %s\n", buffer, contentType, filename, fileExt);
    } else {
        printf("This is not a GET request.\n");
    }

    //reply to client
    // n = write(newsockfd, "I got your message", 18);
    // if (n < 0) error("ERROR writing to socket");

    close(newsockfd);  // close connection
    close(sockfd);

    return 0;
}


