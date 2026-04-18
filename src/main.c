#include <webreq.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void get_input(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (size > 0) {
        buffer[0] = '\0';
    }
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
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
        printf("Processing argument: %s\n", arg);
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
    WebReq_Params params = {0, NULL, NULL, NULL, NULL};

    printf("\n");
    printf("  __      __   _      ___            \n");
    printf("  \\ \\    / /__| |__  | _ \\___ __ _   \n");
    printf("   \\ \\/\\/ / -_) '_ \\ |   / -_) _` |  \n");
    printf("    \\_/\\_/\\___|_.__/ |_|_\\___\\__, |  \n");
    printf("                                |_|  \n");
    printf("\n");

    if (!get_args(argc, argv, &params.port, &params.host, &params.path, &params.message, &params.method))
        return 0;    

#if defined(_WIN32) || defined(_WIN64)
    webreq_init();
#endif

    webreq_make(&params);

    return 0;
}