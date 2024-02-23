#include <parser.h>

void add_get_param(http_request *request, char *key, char *value)
{
    hashmap_put(request->getParams, key, value);
}

void parser_parse_query_string(char *query, http_request *request)
{
    // Ensure there's a query to parse.
    if (query == NULL || *query == '\0') {
        return;
    }

    char *current = query;
    char *next;
    char *saveptr = NULL; // Single saveptr for consistent tokenization context

    // Use a loop to iterate over each parameter-value pair
    while ((next = strchr(current, '&')) != NULL || *current) {
        if (next != NULL) {
            *next = '\0'; // Temporarily end the string to isolate the current part
        }

        // Extract parameter and value
        char *param = strtok_r(current, "=", &saveptr);
        char *value = strtok_r(NULL, "=", &saveptr); // Continue with the same saveptr

        if (param && value) {
            add_get_param(request, param, value);
        }

        if (next != NULL) {
            *next = '&'; // Restore the '&' character if we modified the string
            current = next + 1;
        } else {
            break; // No more parameters to process
        }
    }

    return;
}

void parser_parse_request(char *req, http_request *request)
{
    const char *currentLine = req;
    char *nextLine;
    int lineSize;

    nextLine = strchr(currentLine, '\r');
    if (nextLine == NULL)
    {
        request->error = 1;
        return;
    }
    lineSize = nextLine - currentLine;

    sscanf(currentLine, "%s %s %s", request->method, request->uri, request->version);

    // split query string from uri
    char *queryStart = strchr(request->uri, '?');
    if (queryStart)
    {
        *queryStart = '\0'; // Terminate URI
        request->_query_string = strdup(queryStart + 1);
        parser_parse_query_string(request->_query_string, request);
    }
    else
    {
        request->_query_string = NULL;
        request->params = NULL;
    }

    currentLine = nextLine + 2;

    while ((nextLine = strstr(currentLine, "\r\n")) && (currentLine[0] != '\r'))
    {
        lineSize = nextLine - currentLine;
        char lineBuffer[lineSize + 1];
        strncpy(lineBuffer, currentLine, lineSize);
        lineBuffer[lineSize] = '\0';

        char key[lineSize + 1], value[lineSize + 1];
        sscanf(lineBuffer, "%[^:]: %[^\r\n]", key, value);

        if (strlen(key) == 0 || strlen(value) == 0)
        {
            request->error = 1;
            return;
        }

        // if key is "Cookie", parse cookie
        if (strcmp(key, "Cookie") == 0)
        {
            char *rest; // Pointer needed by strtok_r for its internal state
            char *cookie = strtok_r(value, ";", &rest);
            while (cookie)
            {
                while (*cookie == ' ')
                    cookie++; // Skip leading spaces before cookie name

                char *innerRest; // Separate state pointer for cookie name/value parsing
                char *cookieName = strtok_r(cookie, "=", &innerRest);
                if (cookieName)
                {
                    while (*cookieName == ' ')
                        cookieName++; // Skip leading spaces in cookie name
                    char *cookieValue = strtok_r(NULL, "=", &innerRest);
                    if (cookieValue)
                    {
                        while (*cookieValue == ' ')
                            cookieValue++; // Skip leading spaces in cookie value

                        // TODO: trimming spaces or decoding URL encoding if necessary
                        hashmap_put(request->cookies, cookieName, cookieValue);
                    }
                }

                cookie = strtok_r(NULL, ";", &rest);
            }
        }
        else
        {
            hashmap_put(request->headers, key, value);
        }

        currentLine = nextLine + 2;
    }

    if (currentLine[0] == '\r')
    {
        currentLine += 2;
    }

    if (strlen(currentLine) > 0)
    {
        request->body = strdup(currentLine);
    }
    else
    {
        request->body = NULL;
    }

    request->error = 0;

    return;
}
