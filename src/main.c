#include <server.h>
#include <http.h>
#include <routing.h>
#include <parser.h>


http_response *handle_index(http_request *req)
{
    (void) req;
    return http_response_view(200, "hello", NULL);
}

int main()
{
    router_t *router = router_create();
    router_get(router, "/", handle_index);

    init_server(80, router);
    start_server();
    return 0;
}
