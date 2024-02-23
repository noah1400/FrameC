#include <parser.h>

void parser_parse_query_string(char *query, http_request *request)
{
    // extracts parameters and values from query string
    // and stores them in a hashmap
    char *current = query;
    char *next;
    while ((next = strchr(current, '&')) && (current[0] != '\0'))
    {
        char *param = strtok(current, "=");
        char *value = strtok(NULL, "&");
        hashmap_put(request->getParams, param, value);
        current = next + 1;
    }
    char *param = strtok(current, "=");
    char *value = strtok(NULL, "&");
    hashmap_put(request->getParams, param, value);

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
        request->getParams = hashmap_new();
        parser_parse_query_string(request->_query_string, request);
    }
    else
    {
        request->_query_string = NULL;
        request->params = NULL;
    }

    currentLine = nextLine + 2;

    request->headers = hashmap_new();
    request->cookies = hashmap_new();

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
