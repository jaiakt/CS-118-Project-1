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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


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
    const int BUFF_SIZE = 1024;
    char buffer[BUFF_SIZE + 1];
    char method[128];
    char fileExt[56];

    memset(filename, 0, 256);  // reset memory
    memset(contentType, 0, 128);  // reset memory
    memset(buffer, 0, BUFF_SIZE);  // reset memory
    memset(method, 0, 128);  // reset memory
    memset(fileExt, 0, 56);  // reset memory
    

    //read client's message
    n = read(newsockfd, buffer, BUFF_SIZE);
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

    strcpy(buffer, filename);

    int i = 0;
    int j = 0;
    while (buffer[i] != '\0') {
        if (buffer[i] == '%' && buffer[i+1] == '2' && buffer[i+2] == '0') {
            filename[j] = ' ';
            i += 3; ++j;
        }
        else {
            filename[j] = buffer[i];
            ++i; ++j;
        }
    }

    char filesizeStr[7]; 

    struct stat filestat;
    int filefd = open(filename, O_RDONLY);
    FILE *filefp;
    if (filefd < -1 || fstat(filefd, &filestat) < 0) {
        // Hard coded length of 404 page in bytes
        strcpy (buffer, "HTTP/1.1 404 Not Found\r\nContent-Length: 1246\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n");
        filefp = fopen ("400.html", "r");
    }
    else {
        sprintf(filesizeStr, "%zd", filestat.st_size);
        filefp = fopen (filename, "r");
        sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Length: %s\r\nContent-Type: %s\r\nConnection: keep-alive\r\n\r\n", filesizeStr, contentType);
    }
    close(filefd);
    printf("%s", buffer);

    write (newsockfd, buffer, strlen(buffer));

    int filesize = filestat.st_size + 1;
    int bytesRead;

    while (filesize > 0) {
        bytesRead = fread (buffer, sizeof(char), BUFF_SIZE, filefp);
        filesize -= BUFF_SIZE;
        write (newsockfd, buffer, bytesRead);
    }
    fclose(filefp);

    close(newsockfd);  // close connection
    close(sockfd);

    return 0;
}


