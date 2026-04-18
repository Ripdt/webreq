#ifndef WEB_REQUEST_H
#define WEB_REQUEST_H

#if defined(_WIN32) || defined(_WIN64)
  #define WEBREQ_FUNC __declspec(dllexport)
#else
  #define WEBREQ_FUNC __attribute__((visibility("default")))
#endif

typedef struct {
    int port;
    char *host;
    char *path;
    char *message;
    char *method;
} WebReq_Params;

#if defined(_WIN32) || defined(_WIN64)
WEBREQ_FUNC void webreq_init();
#endif

WEBREQ_FUNC int webreq_make(WebReq_Params* params);

#endif