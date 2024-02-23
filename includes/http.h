#ifndef _http_h
#define _http_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hash.h>

typedef struct {
    char method[10];
    char uri[4096];
    char version[10];

    char *body;
    size_t body_length;

    hashmap_map *headers;

    char *_query_string;
    size_t query_string_length;

    hashmap_map *params;
    int param_count;

    int error;
} http_request;

// struct http_response
typedef struct {
    char version[10];
    int status_code;
    char status_message[50];
    char *body;
    size_t body_length;
    hashmap_map *headers;
} http_response;

char *http_request_to_string(http_request *request);
void http_free_request(http_request *request);

http_response *http_create_response(int status_code, char *status_message, char *body);
char *http_response_to_string(http_response *response);
void http_free_response(http_response *response);

int http_response_add_header(http_response *response, char *key, char *value);
int http_request_add_header(http_request *req, char *key, char *value);
char *http_request_get_header_value(http_request *req, char *key);

char *http_request_get_param(http_request *req, char *key);

char *http_response_status_message(int status_code); // maps status code to status message
http_response *http_response_json(int status_code, char *json_data);
http_response *http_response_text(int status_code, char *text);
http_response *http_response_redirect(char *path);
void http_response_set_cookie(http_response *resp, char *name, char *value, char *path, int max_age);
/*
    Calls http_response_text with the status code and error message
    It does the same thing as http_response_text but it's more readable
*/
http_response *http_response_error(int status_code, char *error_message);


#endif