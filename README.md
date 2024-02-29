# FrameC

A lightweight C-based web framework featuring an integrated web server and simplistic routing mechanism.

## Current Limitations and Known Issues
**HTTP Version Support**: This software currently supports only HTTP/1.1. Compatibility with other versions of the HTTP protocol, such as HTTP/2 or HTTP/1.0, is not guaranteed. Any Encryption using SSL certificates are also not supported. If needed see [Proxy config](#proxy-configurations)

**Request Validation**: Please be aware that HTTP/TCP requests that do not adhere to the expected format may lead to unpredictable behavior. The error handling mechanisms in place may not sufficiently identify or mitigate issues arising from improperly formatted requests.

**Error Checking**: The error detection capabilities are in development, and as such, the system might not accurately recognize all instances of malformed requests. I am trying to actively work to enhance the robustness of the request parsing and validation processes.

**Usage Advisory**: Given the above limitations, I advise users to ensure that input data is well-formed and compliant with HTTP/1.1 specifications to avoid potential issues. I appreciate feedback and bug reports from you to help improve the software.

**Compression Support**: The implemented webserver does not support any encoding. If needed see [Proxy config](#proxy-configurations)

## Proxy configurations

Since the implementation of the webserver does not support any encryption or encoding, you can set up a proxy to handle it.

## Build
```bash
make
```

## Run

```bash
sudo ./bin/server
```

Stopping server with `CTRL+C` will automatically free all remaining resources.

## Usage

```c
#include <server.h>
#include <routing.h>
#include <framec.h>

http_response *handle_index()
{
    hashmap_map *context = hashmap_new();
    hashmap_put(context, "title", "Hello, World!");
    hashmap_put(context, "name", "Noah Scholz");
    http_response *r = http_response_view(200, "index", context);
    hashmap_free(context);
    return r;
}

// GET /hello
http_response *handle_hello() {
    return http_response_text(200, "Hello, World! <GET>");
}

// POST /hello
http_response *handle_hello_post() {
    return http_response_text(200, "Hello, World! <POST>");
}

// GET /hello/{id}
http_response *handle_hello_id() {
    char *id = framec_request("id","nullID");
    char *response = malloc(strlen(id) + 8);
    sprintf(response, "Hello %s!", id);
    http_response *res = http_response_text(200, response);
    free(response);
    return res;
}

// GET /multiple/{id}/{name}
http_response *handle_multiple() {
    char *id = framec_request("id", "nullID");
    char *name = framec_request("name", "nullName");
    char *response = malloc(strlen(id) + strlen(name) + 3);
    sprintf(response, "%s %s", id, name);
    http_response *res = http_response_text(200, response);
    free(response);
    return res;
}

int main()
{
    router_t *router = router_create();
    router_get(router, "/", &handle_index);
    router_get(router, "/hello", &handle_hello);
    router_post(router, "/hello", &handle_hello_post);
    router_get(router, "/hello/{id}", &handle_hello_id);
    router_get(router, "/multiple/{id}/{name}", &handle_multiple);

    init_server(80, router);
    start_server();
    return 0;
}
```

## Route Parameters and Cookies

This section details the methodology for passing parameters to a route in your application and outlines how to work with cookies.

### Parameters

Parameters can be passed to a route using two methods:

#### 1. URI (Path Parameters)

Path parameters are included directly in the URI and are mandatory. If a required parameter is missing, the server returns a `404 Not Found` response.

##### Example URI with Path Parameter:

```txt
/hello/{id}
```

#### 2. Query String Parameters

Alternatively, parameters can be passed via the query string.

##### Example Query String:

```txt
/hello?id=4
```

##### Accessing Parameters

You can access a parameter by calling the `framec_request(char *key, char *default)` function, which looks up uri and query parameters using the given key. if the key is not found the default value is beeing returned.

### Cookies

Accessing Cookies

Cookies can be retrieved from a request using:

```c
http_cookie *cookie = http_request_get_cookie(req, "key");
```

**Function prototype**

```c
http_cookie *http_request_get_cookie(http_request *req, char *key);
```

Setting Cookies

Cookies can be set in a response with:

```c
http_response_set_cookie(res, "name", "value", "/", 3600);
```

**Function prototype**

```c
void http_response_set_cookie(http_response *resp, char *name, char *value, char *path, int max_age);
```

Currently, when setting a cookie, only the `name`, `value`, `path`, and `max_age` attributes are supported.

Cookie Attributes

When accessing a cookie, the following attributes are available:

```c
typedef struct {
    char* name;      // Cookie name
    char* value;     // Cookie value
    char* domain;    // Domain
    char* path;      // Path
    char* expires;   // Expires
    bool secure;     // Secure
    bool httpOnly;   // HttpOnly
    char* sameSite;  // SameSite
} http_cookie;
```