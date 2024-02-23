#ifndef _PARSER_H
#define _PARSER_H

#include <hash.h>
#include <http.h>

void parser_parse_request(char *req, http_request *request);



#endif