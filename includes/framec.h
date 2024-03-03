#ifndef _FRAMEC_H
#define _FRAMEC_H

#include <http.h>
#include <routing.h>
#include <pthread.h>
#include <server.h>
#include <http.h>
#include <session.h>

// Framework control structure
typedef struct _framec_t{
    http_request *request; // current request
    http_response *response; // response that will be sent
    session_t *session; // current session
} framec_t;


void framec_start(); // Start the framework
void framec_terminate(); // Terminate the framework
void framec_handle(int sock, char *r); // takes raw request and handles it


// facade
void framec_response_set_header(char *key, char *value);
void framec_response_set_status(int status);

http_request *framec_get_request();

char *framec_request(char *key, char *def);

void framec_session_set(char *key, char *value);
char *framec_session_get(char *key, char *def);

#endif // _FRAMEC_H