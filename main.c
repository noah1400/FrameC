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

int main() {
    router_t *router = router_create();
    router_get(router, "/", handle_index);
    router_get(router, "/hello", handle_hello);
    router_post(router, "/hello", handle_hello_post);

    init_server(80, router);
    start_server();
    return 0;
}
