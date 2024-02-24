#include <server.h>
#include <http.h>
#include <routing.h>
#include <parser.h>


http_response *handle_index(http_request *req)
{
    (void) req;
    hashmap_map *context = hashmap_new();
    hashmap_put(context, "title", "Hello, World!");
    hashmap_put(context, "name", "Noah Scholz");
    http_response *r = http_response_view(200, "index", context);
    hashmap_free(context);
    return r;
}

http_response *handle_hello(http_request *req)
{
    (void) req;
    return http_response_text(200, "Hello, World!");
}

int main()
{
    router_t *router = router_create();
    router_get(router, "/", handle_index);
    router_get(router, "/hello", handle_hello);

    init_server(80, router);
    start_server();
    return 0;
}
