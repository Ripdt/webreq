#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#define DOMAIN AF_INET                 // IPv4
#define COMMUNICATION_TYPE SOCK_STREAM // TCP
#define PROTOCOL IPPROTO_TCP

void error(const char *msg) { perror(msg); exit(0); }

void get_input(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
    }
}

int get_args(int argc, char *argv[], int *port, char **host, char **path, char **message, char **method) {
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help          Show this help message and exit\n");
            printf("  -H, --host HOST     Specify the host address\n");
            printf("  -p, --port PORT     Specify the port number\n");
            printf("  -P, --path PATH     Specify the output path\n");
            printf("  -m, --message MSG   Specify the message to send\n");
            printf("  -M, --method METHOD Specify the request method (GET/POST)\n");
            return 0;
        }
        
        if (strcmp(arg, "-p") == 0 || strcmp(arg, "--port") == 0) {
            *port = atoi(argv[++i]);
        }
        else if (strcmp(arg, "-H") == 0 || strcmp(arg, "--host") == 0) {
            *host = argv[++i];
        }
        else if (strcmp(arg, "-P") == 0 || strcmp(arg, "--path") == 0) {
            *path = argv[++i];
        }
        else if (strcmp(arg, "-m") == 0 || strcmp(arg, "--message") == 0) {
            *message = argv[++i];
        }
        else if (strcmp(arg, "-M") == 0 || strcmp(arg, "--method") == 0) {
            *method = argv[++i];
        }
    }
    
    printf("===================\n");
    printf("Configuration\n");
    printf("===================\n");
    if (*port == 0) {
        char buf[32];
        get_input("Port: ", buf, sizeof(buf));
        *port = atoi(buf);
    }
    else {
        printf("Port: %d\n", *port);
    }

    if (*host == NULL) {
        char buf[256];
        get_input("Host: ", buf, sizeof(buf));
        *host = strdup(buf);
    }
    else {
        printf("Host: %s\n", *host);
    }

    if (*path == NULL) {
        char buf[256];
        get_input("Path: ", buf, sizeof(buf));
        *path = strdup(buf);
    }
    else {
        printf("Path: %s\n", *path);
    }
    
    if (*message == NULL) {
        char buf[256];
        get_input("Message: ", buf, sizeof(buf));
        *message = strdup(buf);
    }
    else {
        printf("Message: %s\n", *message);
    }

    if (*method == NULL) {
        char buf[16];
        get_input("Method (GET/POST): ", buf, sizeof(buf));
        *method = strdup(buf);
    }
    else {
        printf("Method: %s\n", *method);
    }

    printf("\n");

    return 1;
}

int main(int argc, char *argv[]) {
    int port = 0;
    char *host = NULL, *path = NULL, *message = NULL, *method = NULL;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char response[4096];
    char request[4096];

#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        error("ERROR WSAStartup failed");
#endif        
    printf("\n");
    printf("  __      __   _      ___            \n");
    printf("  \\ \\    / /__| |__  | _ \\___ __ _   \n");
    printf("   \\ \\/\\/ / -_) '_ \\ |   / -_) _` |  \n");
    printf("    \\_/\\_/\\___|_.__/ |_|_\\___\\__, |  \n");
    printf("                                |_|  \n");
    printf("\n");

    if (!get_args(argc, argv, &port, &host, &path, &message, &method))
        return 0;    
        
    /* strip http:// or https:// prefix from host for DNS lookup */
    if (strncmp(host, "http://", 7) == 0) host += 7;
    else if (strncmp(host, "https://", 8) == 0) host += 8;

    /* ensure path starts with / */
    if (path[0] != '/') {
        char *new_path = malloc(strlen(path) + 2);
        sprintf(new_path, "/%s", path);
        path = new_path;
    }

    if (message != NULL && strlen(message) > 0) {
        sprintf(request, "%s %s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\n\r\n%s",
                method, path, host, (int)strlen(message), message);
    } else {
        sprintf(request, "%s %s HTTP/1.0\r\nHost: %s\r\n\r\n",
                method, path, host);
    }    
    
    printf("===================\n");
    printf("Request\n");
    printf("===================\n");
    printf("%s\n", request);

    printf("Connecting to %s:%d...\n", host, port);

    sockfd = socket(DOMAIN, COMMUNICATION_TYPE, PROTOCOL);
    if (sockfd < 0) error("ERROR opening socket");

    server = gethostbyname(host);
    if (server == NULL) error("ERROR no such host");

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = DOMAIN;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    printf("Sending request...\n");

    /* send the request */
    total = strlen(request);
    sent = 0;
    do {
#if defined(_WIN32) || defined(_WIN64)
        bytes = send(sockfd, request+sent, total-sent, 0);
#else
        bytes = write(sockfd,request+sent,total-sent);
#endif
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        memset(response+received, 0, total-received);
#if defined(_WIN32) || defined(_WIN64)
        bytes = recv(sockfd, response+received, total-received, 0);
#else
        bytes = read(sockfd,response+received,total-received);
#endif
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    /*
     * if the number of received bytes is the total size of the
     * array then we have run out of space to store the response
     * and it hasn't all arrived yet - so that's a bad thing
     */
    if (received == total)
        error("ERROR storing complete response from socket");

    /* close the socket */
#if defined(_WIN32) || defined(_WIN64)
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif    /* process response */
    printf("Sent %d bytes, received %d bytes\n\n", sent, received);

    printf("===================\n");
    printf("Response\n");
    printf("===================\n");

    /* parse and display response */
    char *header_end = strstr(response, "\r\n\r\n");
    if (header_end) {
        /* status line */
        char *first_line_end = strstr(response, "\r\n");
        if (first_line_end) {
            printf("Status:  %.*s\n", (int)(first_line_end - response), response);
        }

        /* headers */
        printf("-------------------\n");
        printf("Headers:\n");
        printf("-------------------\n");
        char *hdr = first_line_end + 2;
        while (hdr < header_end) {
            char *next = strstr(hdr, "\r\n");
            if (!next) break;
            printf("  %.*s\n", (int)(next - hdr), hdr);
            hdr = next + 2;
        }

        /* body */
        char *body = header_end + 4;
        if (strlen(body) > 0) {
            printf("-------------------\n");
            printf("Body:\n");
            printf("-------------------\n");
            printf("%s\n", body);
        }
    } else {
        printf("%s\n", response);
    }

    printf("===================\n");

    return 0;
}