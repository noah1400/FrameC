#ifndef _http_h
#define _http_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct header_node {
    char *key;
    char *value;
    struct header_node *next;
} header_node;

typedef struct {
    char method[10];
    char uri[50];
    char version[10];

    char *body;
    size_t body_length;

    header_node *headers;

    char *query_string;
    size_t query_string_length;
} http_request;

// struct http_response
typedef struct {
    char version[10];
    int status_code;
    char status_message[50];
    char *body;
    size_t body_length;
    header_node *headers; // Linked list of headers
} http_response;


http_request *http_parse_request(char *request);
char *http_request_to_string(http_request *request);
void http_free_request(http_request *request);

http_response *http_create_response(int status_code, char *status_message, char *body);
char *http_response_to_string(http_response *response);
void http_free_response(http_response *response);

header_node* create_header_node(char *key, char *value);
int http_response_add_header(http_response *response, char *key, char *value);
void http_request_add_header(http_request *req, char *key, char *value);
const char *http_request_get_header_value(http_request *req, const char *key);


http_response *http_response_json(int status_code, char *json_data);
http_response *http_response_text(int status_code, char *text);
http_response *http_response_redirect(char *path);


#endif