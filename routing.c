#include <routing.h>


router_t *router_create() {
    router_t *router = malloc(sizeof(router_t));
    router->table = NULL;
    router->count = 0;
    return router;
}

route_t *route_create() {
    route_t *route = malloc(sizeof(route_t));
    route->path = NULL;
    route->method = ROUTE_METHOD_GET; // Default to GET
    route->handler = NULL;
    return route;
}

routing_table_t *routing_table_create(route_t *route) {
    routing_table_t *table = malloc(sizeof(routing_table_t));
    table->route = route;
    table->next = NULL;
    return table;
}

void router_add_route(router_t *router, route_t *route) {
    routing_table_t *table = routing_table_create(route);
    if (router->table == NULL) {
        router->table = table;
    } else {
        routing_table_t *current = router->table;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = table;
    }
    router->count++;
}

void router_get(router_t *router, char *path, http_response *(*handler)(http_request *req)) {
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path);
    route->method = ROUTE_METHOD_GET;
    route->handler = handler;
    router_add_route(router, route);
}

void router_post(router_t *router, char *path, http_response *(*handler)(http_request *req)) {
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path); 
    route->method = ROUTE_METHOD_POST;
    route->handler = handler;
    router_add_route(router, route);
}

void router_put(router_t *router, char *path, http_response *(*handler)(http_request *req)) {
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path);
    route->method = ROUTE_METHOD_PUT;
    route->handler = handler;
    router_add_route(router, route);
}

void router_delete(router_t *router, char *path, http_response *(*handler)(http_request *req)) {
    route_t *route = route_create();
    route->path = malloc(strlen(path) + 1);
    strcpy(route->path, path);
    route->method = ROUTE_METHOD_DELETE;
    route->handler = handler;
    router_add_route(router, route);
}

int str_to_method(char *method) {
    if (strcmp(method, "GET") == 0) {
        return ROUTE_METHOD_GET;
    } else if (strcmp(method, "POST") == 0) {
        return ROUTE_METHOD_POST;
    } else if (strcmp(method, "PUT") == 0) {
        return ROUTE_METHOD_PUT;
    } else if (strcmp(method, "DELETE") == 0) {
        return ROUTE_METHOD_DELETE;
    }
    return -1;
}

http_response *router_handle_request(router_t *router, http_request *req) {
    routing_table_t *current = router->table;
    while (current != NULL) {
        if (strcmp(current->route->path, req->uri) == 0 && current->route->method == str_to_method(req->method)) {
            return current->route->handler(req);
        }
        current = current->next;
    }
    return http_response_json(404, "{\"error\": \"Not Found\"}");
}

void route_free(route_t *route)
{
    free(route->path);
    free(route);
}

void router_free(router_t *router) {
    routing_table_t *current = router->table;
    while (current != NULL) {
        routing_table_t *next = current->next;
        route_free(current->route);
        free(current);
        current = next;
    }
    free(router);
}