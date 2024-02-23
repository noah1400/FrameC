#include <server.h>
#include <http.h>
#include <routing.h>
#include <parser.h>

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

// GET /set-cookie
http_response *handle_set_cookie(http_request *req) {
    (void)req; // Unused parameter (req) warning
    http_response *res = http_response_text(200, "Cookie set");
    http_response_set_cookie(res, "name", "value", "/", 3600);
    return res;
}

int main() {
    router_t *router = router_create();
    router_get(router, "/", handle_index);
    router_get(router, "/hello", handle_hello);
    router_post(router, "/hello", handle_hello_post);
    router_get(router, "/hello/{id}", handle_hello_id);
    router_get(router, "/multiple/{id}/{name}", handle_multiple);
    router_get(router, "/set-cookie", handle_set_cookie);

    init_server(80, router);
    start_server();

    // char *request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\nHundesohn\r\n\r\n";
    // parse(request);

    return 0;
}
