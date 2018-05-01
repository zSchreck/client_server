#include <stdio.h> 
#include <string.h> 
#include <time.h> // Used for GMT time creation
#include <sys/socket.h>    
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h> // Used for gethostbyname

int main(int argc, char* argv[]) {
    int i = 0;
    int h = 0;
    int d = 0;
    char* time_arg;
    char* url;
    char* host = malloc(50 * sizeof(char));
    char* file = malloc(200 * sizeof(char));
    int portn = 80;

    // Code to check flags passed in 
    for (i = 0; i < argc; i++) {
        if (strcmp("-h", argv[i]) == 0) {
            h = 1;
            continue;
        }
        if (strcmp("-d", argv[i]) == 0) {
            d = 1;
            time_arg = argv[i + 1];
            continue;
        }
    }
    url = argv[argc - 1];
    char* newurl = url+7; //Strip http off the front of the url

    
    int hasport = sscanf(newurl, "%50[^/:]:%4d/%200[^\n]", host, &portn, file); //parse the url into host, port number, and file using regex
    
    if (hasport < 3) {
        // If previous sscanf returned < 3, that means we were missing a port, so parse without that using regex
        sscanf(newurl, "%50[^/]/%200[^\n]", host, file);
    }

    // Code for time formatting given in pdf
    int day, hour, minute;
    char * s_time = malloc(30 * sizeof(char));
    if (d == 1){
        sscanf(time_arg, "%4d:%4d:%4d", &day, &hour, &minute);
        time_t n_time;
        n_time=time(0);
        n_time=n_time-(day*24*3600+hour*3600+minute*60);
        strcpy(s_time, ctime(&n_time)); 
    }
        
    // Code to resolve ip address of the hostname that is passed in
    char ip[50];
    struct hostent *hostinfo;
    struct in_addr **addresslist;
    hostinfo = gethostbyname(host);
    addresslist = (struct in_addr **) hostinfo->h_addr_list;

    for (i = 0; addresslist[i] != NULL; i++) {
        strcpy(ip, inet_ntoa(*addresslist[i]));
    }

    struct sockaddr_in server;
    // Open a socket that we will connect to 
    int sockt = socket(AF_INET, SOCK_STREAM, 0);

    if (sockt < 0) {
        printf("Could not open a socket");
        return 1;
    }

    // Specifying ip address, port number, and protocol to use when connecting to our socket
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(portn);

    // Try connecting to our socket, if it fails return 1
    if (connect(sockt, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printf("could not connect to socket");
        return 1;
    }
    
    char* request = malloc(2000 * sizeof(char));
    char* newhost = malloc(100 * sizeof(char));
    // Add port number to end of hostname, defaulting to 80 if nothing was specified
    sprintf(newhost, "%s:%d", host, portn);
    if (h == 1) {
        // If h was passed, generate a HEAD http request
        sprintf(request, "HEAD %s HTTP/1.1\r\n" "Host: %s\r\n" "\r\n", file, newhost);
    } else if (d == 1) {
        // If d was passed, generate a GET with If-Modified-Since
        sprintf(request, "GET %s HTTP/1.1\r\n" "Host: %s\r\nIf-Modified-Since: %s\r\n" "\r\n", file, newhost, s_time);
    } else {
        // Otherwise, generate a regular GET request
        sprintf(request, "GET %s HTTP/1.1\r\n" "Host: %s\r\n" "\r\n", file, newhost);
    }
    
    // Send our prepared http request on the socket we opened earler
    send(sockt, request, strlen(request), 0);

    // Open a file to be written to
    FILE* filepointer = fopen("response", "w");
    // Create a response buffer, one char longer than our mas response length to allow for null-terminater
    char response[1001];
    // Loop to keep reading while there is data available
    while (1) {
        // Clear our response buffer before writing to it to avoid junk
        memset(response, 0, sizeof(response));
        int length = recv(sockt, response, 1000, 0);
        fputs(response, filepointer);
        // If length is less than 1000, we have read the last of the data, so we can end the loop
        if (length < 1000)
            break;
    }

    // Close the file pointer and the socket.
    fclose(filepointer);
    close(sockt);

    return 0;
}