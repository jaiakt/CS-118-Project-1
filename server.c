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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


void error(char *msg)
{
    perror(msg);
    exit(1);
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
    char contentType[100];
    const int BUFF_SIZE = 1024;
    char buffer[BUFF_SIZE + 1];
    char method[128];

    memset(buffer, 0, BUFF_SIZE);  // reset memory

    //read client's message
    n = read(newsockfd, buffer, BUFF_SIZE);
    if (n < 0) error("ERROR reading from socket");
    printf("%s", buffer);

    memcpy(method, buffer, 4);
    ret = strcmp(method, "GET ");
    if (ret == 0) {
        printf("This is a GET request.\n");
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
