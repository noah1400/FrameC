#include <routing.h>

router_t *router_create()
{
    router_t *router = malloc(sizeof(router_t));
    router->table = NULL;
    router->count = 0;
    return router;
}

route_t *route_create()
{
    route_t *route = malloc(sizeof(route_t));
    route->path = NULL;
    route->method = ROUTE_METHOD_GET; // Default to GET
    route->handler = NULL;
    return route;
}

routing_table_t *routing_table_create(route_t *route)
{
    routing_table_t *table = malloc(sizeof(routing_table_t));
    table->route = route;
    table->next = NULL;
    return table;
}

void router_add_route(router_t *router, route_t *route)
{
    routing_table_t *table = routing_table_create(route);
    if (router->table == NULL)
    {
        router->table = table;
    }
    else
    {
        routing_table_t *current = router->table;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = table;
    }
    router->count++;
}

void router_get(router_t *router, char *path, http_response *(*handler)(http_request *req))
{
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path);
    route->method = ROUTE_METHOD_GET;
    route->handler = handler;
    router_add_route(router, route);
}

void router_post(router_t *router, char *path, http_response *(*handler)(http_request *req))
{
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path);
    route->method = ROUTE_METHOD_POST;
    route->handler = handler;
    router_add_route(router, route);
}

void router_put(router_t *router, char *path, http_response *(*handler)(http_request *req))
{
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path);
    route->method = ROUTE_METHOD_PUT;
    route->handler = handler;
    router_add_route(router, route);
}

void router_delete(router_t *router, char *path, http_response *(*handler)(http_request *req))
{
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path);
    route->method = ROUTE_METHOD_DELETE;
    route->handler = handler;
    router_add_route(router, route);
}

int str_to_method(char *method)
{
    if (strcmp(method, "GET") == 0)
    {
        return ROUTE_METHOD_GET;
    }
    else if (strcmp(method, "POST") == 0)
    {
        return ROUTE_METHOD_POST;
    }
    else if (strcmp(method, "PUT") == 0)
    {
        return ROUTE_METHOD_PUT;
    }
    else if (strcmp(method, "DELETE") == 0)
    {
        return ROUTE_METHOD_DELETE;
    }
    return -1;
}

http_response *router_handle_request(router_t *router, http_request *req)
{
    route_t *route = match_route(req, router);
    if (route != NULL)
    {
        return route->handler(req);
    }
    return http_response_json(404, "{\"error\": \"Not Found\"}");
}

void route_free(route_t *route)
{
    free(route->path);
    free(route);
}

void router_free(router_t *router)
{
    routing_table_t *current = router->table;
    while (current != NULL)
    {
        routing_table_t *next = current->next;
        route_free(current->route);
        free(current);
        current = next;
    }
    free(router);
}

param_t *create_param(const char *key, const char *value)
{
    param_t *new_param = (param_t *)malloc(sizeof(param_t));
    if (new_param == NULL)
    {
        // Handle memory allocation failure if needed
        return NULL;
    }
    new_param->key = strdup(key);     // Make sure to copy the key
    new_param->value = strdup(value); // Make sure to copy the value
    new_param->next = NULL;           // Next pointer is initially NULL
    return new_param;
}

void add_param_to_request(http_request *req, const char *key, const char *value)
{
    param_t *new_param = create_param(key, value);
    if (new_param == NULL)
    {
        // Handle memory allocation failure if needed
        return;
    }

    if (req->params == NULL)
    {
        // List is empty, new param becomes the head
        req->params = new_param;
    }
    else
    {
        // Append to the end of the list
        param_t *current = req->params;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_param;
    }

    req->param_count++; // Increment the count of parameters
}

char **split_string(const char *str, const char *delimiter, int *count)
{
    char **array = NULL;
    int capacity = 10;
    *count = 0;

    // Allocate initial memory for the array
    array = (char **)malloc(capacity * sizeof(char *));
    if (array == NULL)
    {
        // Handle memory allocation failure
        return NULL;
    }

    char *str_copy = strdup(str);
    if (str_copy == NULL)
    {
        // Handle strdup failure, after freeing already allocated memory
        free(array);
        return NULL;
    }

    char *token = strtok(str_copy, delimiter);
    while (token)
    {
        if (*count >= capacity)
        {
            capacity *= 2;
            char **new_array = realloc(array, capacity * sizeof(char *));
            if (new_array == NULL)
            {
                // Handle realloc failure: free memory and return
                // Free previously duplicated strings in array
                for (int i = 0; i < *count; i++)
                {
                    free(array[i]);
                }
                free(array);
                free(str_copy);
                return NULL;
            }
            array = new_array;
        }
        array[*count] = strdup(token);
        if (array[*count] == NULL)
        {
            // Handle strdup failure: free memory and return
            // Free previously duplicated strings in array and the array itself
            for (int i = 0; i < *count; i++)
            {
                free(array[i]);
            }
            free(array);
            free(str_copy);
            return NULL;
        }
        (*count)++;
        token = strtok(NULL, delimiter);
    }
    free(str_copy);

    return array;
}

route_t *match_route(http_request *req, router_t *router)
{
    routing_table_t *current = router->table;
    while (current != NULL)
    {
        int route_segments_count, uri_segments_count;
        char **route_segments = split_string(current->route->path, "/", &route_segments_count);
        char **uri_segments = split_string(req->uri, "/", &uri_segments_count);

        if (route_segments_count == uri_segments_count)
        {
            int i;
            for (i = 0; i < route_segments_count; i++)
            {
                if (route_segments[i][0] == '{')
                {
                    // Extract parameter name and value, assuming the format is "{paramName}"
                    char param_name[256];
                    strncpy(param_name, route_segments[i] + 1, strlen(route_segments[i]) - 2);
                    param_name[strlen(route_segments[i]) - 2] = '\0';

                    add_param_to_request(req, param_name, uri_segments[i]);
                }
                else if (strcmp(route_segments[i], uri_segments[i]) != 0)
                {
                    break;
                }
            }

            if (i == route_segments_count)
            {
                // Clean up
                for (int j = 0; j < route_segments_count; j++)
                    free(route_segments[j]);
                free(route_segments);
                for (int j = 0; j < uri_segments_count; j++)
                    free(uri_segments[j]);
                free(uri_segments);

                return current->route;
            }
        }

        // Clean up
        for (int j = 0; j < route_segments_count; j++)
            free(route_segments[j]);
        free(route_segments);
        for (int j = 0; j < uri_segments_count; j++)
            free(uri_segments[j]);
        free(uri_segments);

        current = current->next;
    }

    return NULL; // No match found
}