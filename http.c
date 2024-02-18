#include <http.h>

http_request *http_parse_request(char *request)
{

    http_request *req = (http_request *)malloc(sizeof(http_request));
    if (!req)
        return NULL; // Memory allocation failed

    memset(req, 0, sizeof(http_request)); // Initialize the structure

    char *saveptr; // Pointer used by strtok_r to maintain context between calls
    char *line = strtok_r(request, "\r\n", &saveptr);
    if (line != NULL)
    {
        sscanf(line, "%s %s %s", req->method, req->uri, req->version); // Parse request line
        // Now you can continue to use strtok_r with saveptr to tokenize the rest of the request
    }
    else
    {
        // Handle error: request does not contain a valid request line
        printf("Invalid request line\n");
        free(req); // Assuming req was allocated with malloc before
        return NULL;
    }

    char *headersStart = line + strlen(line) + 2;       // Skip past the first request line and \r\n
    char *headerEnd = strstr(headersStart, "\r\n\r\n"); // Find the end of headers

    char *headerLine;
    while ((headerLine = strtok_r(NULL, "\r\n", &saveptr)) && *headerLine)
    {
        char *colon = strchr(headerLine, ':');
        if (!colon)
            continue; // Malformed header line, skip

        *colon = '\0'; // Temporarily terminate the key string
        char *key = headerLine;
        char *value = colon + 1;
        while (*value == ' ')
            value++; // Skip leading whitespaces in value

        http_request_add_header(req, key, value);
    }

    // Separate query string from URI if present
    char *queryStart = strchr(req->uri, '?');
    if (queryStart)
    {
        *queryStart = '\0'; // Null-terminate the URI at the '?' character
        if (queryStart && *(queryStart + 1) != '\0')
        {
            *queryStart = '\0';
            req->query_string = strdup(queryStart + 1);
            req->query_string_length = strlen(req->query_string);
        }
        else
        {
            // Handle the case where '?' is the last character or there's no query string
            printf("No query string\n");
        }
        req->query_string_length = strlen(req->query_string);
    }

    const char *contentLengthStr = http_request_get_header_value(req, "Content-Length");
    if (contentLengthStr)
    {
        int contentLength = atoi(contentLengthStr);
        if (contentLength > 0)
        {
            // Calculate the actual start of the body
            char *bodyStart = headerEnd + 4; // Skip past the \r\n\r\n that marks the end of headers
            size_t actualLength = strlen(bodyStart);
            size_t bodyLength = (size_t)contentLength < actualLength ? (size_t)contentLength : actualLength;

            req->body = (char *)malloc(bodyLength + 1); // Allocate memory for the body
            if (req->body)
            {
                strncpy(req->body, bodyStart, bodyLength);
                req->body[bodyLength] = '\0'; // Null-terminate the body string
                req->body_length = bodyLength;
            }
        }
    }
    else
    {
        // If Content-Length header is not present or invalid, you might choose to handle this differently.
        // For simplicity, we'll assume there's no body or handle it as your original approach.
    }

    return req;
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