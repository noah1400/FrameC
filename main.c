#include <server.h>
#include <http.h>
#include <routing.h>

// GET /
http_response *handle_index(http_request *req) {
    (void)req; // Unused parameter (req) warning
    return http_response_text(200, "Index <GET>");
}

// GET /hello
http_response *handle_hello(http_request *req) {
    (void)req; // Unused parameter (req) warning
    return http_response_text(200, "Hello, World! <GET>");
}

// POST /hello
http_response *handle_hello_post(http_request *req) {
    (void)req; // Unused parameter (req) warning
    return http_response_text(200, "Hello, World! <POST>");
}

// GET /hello/{id}
http_response *handle_hello_id(http_request *req) {
    char *id = http_request_get_param(req, "id");
    char *response = malloc(strlen(id) + 8);
    sprintf(response, "Hello %s!", id);
    http_response *res = http_response_text(200, response);
    free(response);
    return res;
}

// GET /multiple/{id}/{name}
http_response *handle_multiple(http_request *req) {
    char *id = http_request_get_param(req, "id");
    char *name = http_request_get_param(req, "name");
    char *response = malloc(strlen(id) + strlen(name) + 3);
    sprintf(response, "%s %s", id, name);
    http_response *res = http_response_text(200, response);
    free(response);
    return res;
}

int main() {
    router_t *router = router_create();
    router_get(router, "/", handle_index);
    router_get(router, "/hello", handle_hello);
    router_post(router, "/hello", handle_hello_post);
    router_get(router, "/hello/{id}", handle_hello_id);
    router_get(router, "/multiple/{id}/{name}", handle_multiple);

    init_server(80, router);
    start_server();
    return 0;
}
