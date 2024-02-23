#ifndef _PARSER_H
#define _PARSER_H

#include <mpc.h>
#include <hash.h>

struct parsed_object {
    char *method;
    char *uri;
    char *version;
    char *body;
    char *query_string;
    char *headers;
    char *params;
    int param_count;
    int error;
};

typedef struct parsed_object parsed_object;

parsed_object *parse(char *input);

#endif