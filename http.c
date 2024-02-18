#include <http.h>

http_request *http_parse_request(char *request)
{
    http_request *req = (http_request *)malloc(sizeof(http_request));
    if (!req)
        return NULL; // Memory allocation failed

    memset(req, 0, sizeof(http_request)); // Initialize the structure

    char *saveptr; // For strtok_r context
    char *line = strtok_r(request, "\r\n", &saveptr);
    if (!line)
    {
        printf("Invalid request line\n");
        printf("Request: %s\n\n", request);
        free(req);
        return NULL;
    }

    parse_request_line(&line, req);
    if (req->error)
    {
        printf("Error parsing request: \n\n%s\n\n", request);
        free(req);
        return NULL;
    }
    parse_headers(&saveptr, req);
    separate_query_string(req);

    // Find end of headers to determine start of the body
    char *headersEnd = strstr(line, "\r\n\r\n");
    if (headersEnd)
    {
        parse_body(headersEnd, req);
    }

    return req;
}

void parse_request_line(char **line, http_request *req)
{
    // Define maximum sizes for method, uri, and version to prevent overflow
    char method[8], uri[1024], version[16];
    int parsed = sscanf(*line, "%7s %1023s %15s", method, uri, version); // Limit input size
    if (parsed != 3)
    {
        // Handle error: Invalid request line
        fprintf(stderr, "Error parsing request line\n");
        req->error = 1;
        return; // Consider how you want to handle this failure (e.g., setting an error flag in req)
    }
    // Ensure the strings are safely copied into the http_request structure
    strncpy(req->method, method, sizeof(req->method) - 1);
    req->method[sizeof(req->method) - 1] = '\0';
    strncpy(req->uri, uri, sizeof(req->uri) - 1);
    req->uri[sizeof(req->uri) - 1] = '\0';
    strncpy(req->version, version, sizeof(req->version) - 1);
    req->version[sizeof(req->version) - 1] = '\0';
}

void parse_headers(char **saveptr, http_request *req)
{
    char *line;
    while ((line = strtok_r(NULL, "\r\n", saveptr)) && *line)
    {
        char *colon = strchr(line, ':');
        if (!colon)
            continue; // Skip malformed header

        *colon = '\0'; // Temporarily terminate key
        char *key = line;
        char *value = colon + 2; // Skip colon and space

        // Add the parsed header (assuming http_request_add_header is implemented)
        http_request_add_header(req, key, value);
    }
}

void separate_query_string(http_request *req)
{
    char *queryStart = strchr(req->uri, '?');
    if (queryStart)
    {
        *queryStart = '\0'; // Terminate URI
        req->query_string = strdup(queryStart + 1);
        req->query_string_length = strlen(req->query_string);
    }
}

void parse_body(char *headersEnd, http_request *req)
{
    char *bodyStart = headersEnd + 4; // Skip \r\n\r\n
    const char *contentLengthStr = http_request_get_header_value(req, "Content-Length");
    if (contentLengthStr)
    {
        int contentLength = atoi(contentLengthStr);
        size_t bodyLength = (size_t)contentLength;
        req->body = (char *)malloc(bodyLength + 1);
        if (req->body)
        {
            strncpy(req->body, bodyStart, bodyLength);
            req->body[bodyLength] = '\0';
            req->body_length = bodyLength;
        }
    }
}

void http_free_request(http_request *req)
{
    if (req->query_string)
    {
        free(req->query_string);
    }
    header_node *current = req->headers;
    while (current)
    {
        header_node *next = current->next;
        free(current->key);
        free(current->value);
        free(current);
        current = next;
    }
    if (req->body)
    {
        free(req->body);
    }
    free(req);
}

// Assuming http_request is defined as previously described

char *http_request_to_string(http_request *request)
{
    // Calculate the total length needed for the string representation
    size_t totalLength = strlen(request->method) + strlen(request->uri) + strlen(request->version) + 4; // Space for spaces and \r\n
    if (request->query_string)
    {
        totalLength += strlen(request->query_string) + 1; // +1 for '?'
    }

    // Allocate memory for the string representation
    char *requestString = (char *)malloc(totalLength + 1); // +1 for null terminator
    if (!requestString)
    {
        return NULL; // Memory allocation failed
    }

    // Start constructing the string
    char *ptr = requestString;
    ptr += sprintf(ptr, "<%s %s%s%s %s>",
                   request->method,
                   request->uri,
                   request->query_string ? "?" : "",
                   request->query_string ? request->query_string : "",
                   request->version);

    return requestString;
}

http_response *http_create_response(int status_code, char *status_message, char *body)
{
    http_response *response = (http_response *)malloc(sizeof(http_response));
    if (!response)
        return NULL; // Failed to allocate memory for response

    strcpy(response->version, "HTTP/1.1");
    response->status_code = status_code;
    strncpy(response->status_message, status_message, sizeof(response->status_message) - 1);
    response->status_message[sizeof(response->status_message) - 1] = '\0'; // Ensure null-termination

    response->body = body ? strdup(body) : NULL;
    response->body_length = body ? strlen(body) : 0;

    response->headers = NULL; // Initially, no headers

    // Example usage: Add a default Content-Type header
    // This assumes http_response_add_header is properly implemented
    http_response_add_header(response, "Content-Type", "text/plain");
    if (body)
    {
        char contentLength[32];
        sprintf(contentLength, "%zu", response->body_length);
        http_response_add_header(response, "Content-Length", contentLength);
    }

    return response;
}

void http_free_response(http_response *response)
{
    if (response->body)
        free(response->body);
    // Free headers
    header_node *current = response->headers;
    while (current)
    {
        header_node *next = current->next;
        free(current->key);
        free(current->value);
        free(current);
        current = next;
    }
    free(response);
}

char *http_response_to_string(http_response *response)
{
    // Estimate initial size
    size_t totalLength = strlen(response->version) + 4 + strlen(response->status_message) + 2 + response->body_length + 2;

    // Calculate headers length
    for (header_node *current = response->headers; current != NULL; current = current->next)
    {
        totalLength += strlen(current->key) + 2 + strlen(current->value) + 2; // key: value\r\n
    }

    char *responseString = (char *)malloc(totalLength + 1); // +1 for null terminator
    if (!responseString)
        return NULL;

    sprintf(responseString, "%s %d %s\r\n", response->version, response->status_code, response->status_message);

    // Append headers
    for (header_node *current = response->headers; current != NULL; current = current->next)
    {
        strcat(responseString, current->key);
        strcat(responseString, ": ");
        strcat(responseString, current->value);
        strcat(responseString, "\r\n");
    }

    strcat(responseString, "\r\n"); // Headers and body separator

    // Append body if exists
    if (response->body && response->body_length > 0)
    {
        strcat(responseString, response->body);
    }

    return responseString;
}

header_node *create_header_node(char *key, char *value)
{
    header_node *node = (header_node *)malloc(sizeof(header_node));
    if (!node)
        return NULL;

    node->key = strdup(key);
    node->value = strdup(value);
    node->next = NULL;
    return node;
}

int http_response_add_header(http_response *response, char *key, char *value)
{
    header_node **current = &(response->headers);

    // Search for an existing header with the same key
    while (*current)
    {
        if (strcmp((*current)->key, key) == 0)
        {
            // Key found, overwrite its value
            free((*current)->value);           // Free the old value
            (*current)->value = strdup(value); // Set new value
            return 0;                          // Success
        }
        current = &((*current)->next);
    }

    // Key not found, add a new header
    header_node *newNode = create_header_node(key, value);
    if (!newNode)
        return -1; // Memory allocation failed

    *current = newNode; // Append new node to the list
    return 0;           // Success
}

void http_request_add_header(http_request *req, char *key, char *value)
{
    if (!req || !key || !value)
        return; // Basic validation

    // Look for an existing header with the same key
    header_node *current = req->headers;
    while (current)
    {
        if (strcmp(current->key, key) == 0)
        {
            // Found an existing header, update its value
            free(current->value);           // Free the old value
            current->value = strdup(value); // Assign the new value
            return;
        }
        current = current->next;
    }

    // No existing header found, create a new one
    header_node *new_node = create_header_node(key, value);
    if (!new_node)
        return; // Failed to create a new node

    if (req->headers == NULL)
    {
        // The list is empty, make the new node the head
        req->headers = new_node;
    }
    else
    {
        // Find the last node in the list
        current = req->headers;
        while (current->next != NULL)
        {
            current = current->next;
        }
        // Add the new node to the end of the list
        current->next = new_node;
    }
}

const char *http_request_get_header_value(http_request *req, const char *key)
{
    if (!req || !key)
    {
        return NULL; // Invalid input
    }

    header_node *current = req->headers;
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            return current->value; // Found the header, return its value
        }
        current = current->next; // Move to the next header in the list
    }

    return NULL; // Header not found
}