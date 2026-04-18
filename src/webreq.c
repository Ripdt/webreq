#include "webreq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#define DOMAIN AF_INET                 // IPv4
#define COMMUNICATION_TYPE SOCK_STREAM // TCP
#define PROTOCOL IPPROTO_TCP

void error(const char *msg) { perror(msg); exit(0); }

#if defined(_WIN32) || defined(_WIN64)
int webreq_init() 
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        error("ERROR WSAStartup failed");
}
#endif

void format_host(char **host) {
    /* strip http:// or https:// prefix from host for DNS lookup */
    if (strncmp(*host, "http://", 7) == 0) {
        memmove(*host, *host + 7, strlen(*host) - 6);
    } else if (strncmp(*host, "https://", 8) == 0) {
        memmove(*host, *host + 8, strlen(*host) - 7);
    }

    /* remove a trailing slash if provided */
    size_t len = strlen(*host);
    if (len > 0 && (*host)[len - 1] == '/') {
        (*host)[len - 1] = '\0';
    }
}

void format_path(char **path) {
    /* ensure path starts with / */
    if ((*path)[0] != '/') {
        char new_path[256];
        snprintf(new_path, sizeof(new_path), "/%s", *path);
        *path = strdup(new_path);
    }
}

int webreq_make(WebReq_Params* params)
{
#if defined(_WIN32) || defined(_WIN64)
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char response[4096];
    char request[4096];

    format_host(&params->host);
    format_path(&params->path);

    if (params->message != NULL && strlen(params->message) > 0) {
        sprintf(request, "%s %s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\n\r\n%s",
                params->method, params->path, params->host, (int)strlen(params->message), params->message);
    } else {
        sprintf(request, "%s %s HTTP/1.0\r\nHost: %s\r\n\r\n",
                params->method, params->path, params->host);
    }
    
    printf("===================\n");
    printf("Request\n");
    printf("===================\n");
    printf("%s\n", request);

    printf("Connecting to %s:%d...\n", params->host, params->port);

    sockfd = socket(DOMAIN, COMMUNICATION_TYPE, PROTOCOL);
    if (sockfd < 0) error("ERROR opening socket");

    server = gethostbyname(params->host);

    if (server == NULL) error("ERROR no such host");

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = DOMAIN;
    serv_addr.sin_port = htons(params->port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    printf("Sending request...\n");

    /* send the request */
    total = strlen(request);
    sent = 0;
    do {
        bytes = send(sockfd, request+sent, total-sent, 0);
        if (bytes < 0)
            error("ERROR writing params->message to socket");
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
        bytes = recv(sockfd, response+received, total-received, 0);
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

    closesocket(sockfd);
    WSACleanup();

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

    return 1;
#else
    struct addrinfo hints, *res;
    char port_str[6] = {0};
    const char *port = NULL;
    int sockfd, bytes, sent, received, total;
    char response[4096];
    char request[4096];

    format_host(&params->host);
    format_path(&params->path);

    if (params->message != NULL && strlen(params->message) > 0) {
        sprintf(request, "%s %s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\n\r\n%s",
                params->method, params->path, params->host, (int)strlen(params->message), params->message);
    } else {
        sprintf(request, "%s %s HTTP/1.0\r\nHost: %s\r\n\r\n",
                params->method, params->path, params->host);
    }
    
    if (params->port < 0 || params->port > 65535)
        error("ERROR invalid port number");

    if (params->port != 0) {
        snprintf(port_str, sizeof(port_str), "%d", params->port);
        port = port_str;
    } else {
        port = "80";
    }

    printf("===================\n");
    printf("Request\n");
    printf("===================\n");
    printf("%s\n", request);

    printf("Connecting to %s:%s...\n", params->host, port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    int gai_err = getaddrinfo(params->host, port, &hints, &res);
    if (gai_err != 0) {
        fprintf(stderr, "ERROR getaddrinfo: %s\n", gai_strerror(gai_err));
        exit(0);
    }
    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) error("ERROR opening socket");

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
        error("ERROR connecting");

    printf("Sending request...\n");

    /* send the request */
    total = strlen(request);
    sent = 0;
    do {
        bytes = write(sockfd,request+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing params->message to socket");
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
        bytes = read(sockfd,response+received,total-received);
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

    close(sockfd);

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

    freeaddrinfo(res);

    printf("===================\n");

    return 1;
#endif
}