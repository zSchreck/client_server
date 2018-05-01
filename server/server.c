#include <stdio.h> 
#include <string.h> 
#include <time.h> // Used for GMT time creation
#include <sys/socket.h>    
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h> // Used for gethostbyname
#include <pthread.h>
#include <sys/stat.h> // Used for geting file info

void * handle_connection(void *);

int main(int argc, char* argv[]) {
    int sockt, new_sockt, c;

    struct sockaddr_in server, client;

    // Create a socket to bind to later
    sockt = socket(AF_INET, SOCK_STREAM, 0);

    if (sockt < 0) {
        printf("Could not open a socket");
        return 1;
    }

    // Set data for server
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8000);

    // Bind the socket
    int didbind = bind(sockt, (struct sockaddr *)&server, sizeof(server));

    if (didbind < 0) {
        printf("Could not bind socket");
        return 1;
    }

    printf("\n\nServer has been created, running at 127.0.0.1:8000\n\nNow listening for incoming connections...\n\n");
    // Listen for incoming connections
    listen(sockt, 4);

    while ((new_sockt = accept(sockt, (struct sockaddr *)&client, (socklen_t *)&c))) {
        printf("New connection has been accepted");

        pthread_t new_thread;
        int* new_socket = malloc(1);
        *new_socket = new_sockt;

        int create_thread = pthread_create(&new_thread, NULL, handle_connection, (void *) new_socket);
        if (create_thread < 0) {
            printf("Could not create new thread");
            return 1;
        }
    }
}

void * handle_connection(void * sockt) {
    char message[1501];
    memset(message, 0, sizeof(message));
    int socket_loc = *((int*) sockt);
    read(socket_loc, message, 1500);
    printf("\n\n\nMessage recieved on server:\n\n%s\n\n\n\n", message);

    // Flag for determining http request method. Set to 1 if GET, 2 if HEAD, 3 if GET with if-modified-since
    int req_type = -1;
    char* file = malloc(200 * sizeof(char));
    if (strstr(message, "HEAD") != NULL)
        req_type = 2;
    if (strstr(message, "GET") != NULL) {
        req_type = 1;
        if (strstr(message, "If-Modified-Since"))
            req_type = 3;
    }

    if (req_type == 1) {
        file = strstr(message, "GET") + 4;
        // Find the length of the file description
        int filelen = strcspn(file, " ");
        char newfile[150];
        // Extract the file description
        strncpy(newfile, file, filelen);
        char buffer[1001];
        FILE *fp;
        char filename[160] = "./";
        strcat(filename, newfile);
        fp = fopen(filename, "r");
        if (!fp) {
            // File not found return 404
            char* not_found = "HTTP/1.1 404 Not Found\n";
            write(socket_loc, not_found, strlen(not_found));
            close(socket_loc);
            return NULL;
        }
        // File is found, continuously read from file and write to socket until no more reading
        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int reading = fread(buffer, sizeof(char), 1000, fp);
            if (reading == 0) 
                break;
            if (reading < 0) {
                printf("Reading from file failed.");
            }

            int writing = write(socket_loc, buffer, reading);
            if (writing <= 0) 
                printf("Failed to write to socket");

        }
        fclose(fp);
    } else if (req_type == 2) {
        file = strstr(message, "HEAD") + 5;
        // Find the length of the file description
        int filelen = strcspn(file, " ");
        char newfile[150];
        // Extract the file description
        strncpy(newfile, file, filelen);
        FILE *fp;
        char filename[160] = "./";
        strcat(filename, newfile);
        fp = fopen(filename, "r");
        if (!fp) {
            // File not found, return 404
            char* not_found = "HTTP/1.1 404 Not Found\n";
            write(socket_loc, not_found, strlen(not_found));
            close(socket_loc);
            fclose(fp);
            return NULL;
        } else {
            // File found, return 200
            char* not_found = "HTTP/1.1 200 OK\n";
            write(socket_loc, not_found, strlen(not_found));
            close(socket_loc);
            fclose(fp);
            return NULL;
        }
    } else if (req_type == 3) {
        file = strstr(message, "GET") + 4;
        // Find the length of the file description
        int filelen = strcspn(file, " ");
        char newfile[150];
        // Extract the file description
        strncpy(newfile, file, filelen);
        printf("\n\n\nFile: %s\n\n\n", newfile);
        char buffer[1001];
        FILE *fp;
        char filename[160] = "./";
        strcat(filename, newfile);

        fp = fopen(filename, "r");
        if (!fp) {
            // File not found return 404
            char* not_found = "HTTP/1.1 404 Not Found\n";
            write(socket_loc, not_found, strlen(not_found));
            close(socket_loc);
            return NULL;
        }

        struct stat fileinfo;
        
        if (stat(newfile, &fileinfo) < 0) {
            printf("\nStat failed for file: %s\n\n", newfile);
            close(socket_loc);
        }
        long modified = fileinfo.st_mtime;

        char* mtime = strstr(file, "If-Modified-Since") + 19;
        struct tm tm;
        time_t t;
        printf("\n]\npassed in time: %s\n\n", mtime);

        if (strptime(mtime, "%a %b %d %H:%M:%S %Y", &tm) == NULL)
            printf("Could not convert time");
        time_t timer = mktime(&tm);

        if (modified - timer >= 0) {
            while (1) {
                memset(buffer, 0, sizeof(buffer));
                int reading = fread(buffer, sizeof(char), 1000, fp);
                if (reading == 0) 
                    break;
                if (reading < 0) {
                    printf("Reading from file failed.");
                }

                int writing = write(socket_loc, buffer, reading);
                if (writing <= 0) 
                    printf("Failed to write to socket");
            }

            fclose(fp);
            close(socket_loc);
            
        } else {
            // File not modified return 304
            char* not_modified = "HTTP/1.1 304 Not Modified\n";
            write(socket_loc, not_modified, strlen(not_modified));
            close(socket_loc);
            fclose(fp);
            return NULL;
        }
    }
    close(socket_loc);
    return 0;
}