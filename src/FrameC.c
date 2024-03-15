#include <framec.h>

static framec_t *framec_create();
static void framec_destroy();
static void framec_send_response(int sock, framec_t *framec);
static void framec_set_response(framec_t *framec, http_response *response);
static void framec_set_request(framec_t *framec, http_request *request);


static framec_t *get_framec()
{
    return (framec_t *)pthread_getspecific(global_server->frame_key);
}

static framec_t *framec_create()
{
    framec_t *framec = (framec_t *)malloc(sizeof(framec_t));
    if (framec == NULL)
    {
        perror("Could not allocate memory for framec_t");
        framec_terminate();
        exit(1);
    }
    config_t *config = config_new();
    framec->config = config;
    return framec;
}

static void framec_destroy()
{
    framec_t *framec = get_framec();
    if (framec->request)
    {
        http_free_request(framec->request);
    }
    if (framec->response)
    {
        http_free_response(framec->response);
    }
    config_free(framec->config);
    free(framec);
}

static void framec_send_response(int sock, framec_t *framec)
{
    char *response = http_response_to_string(framec->response);

    char *req_str = http_request_to_string(framec->request);
    printf("%s %d s:%ld\r\n", req_str, framec->response->status_code, strlen(response));
    free(req_str);

    ssize_t sent = send(sock, response, strlen(response), 0);
    if (sent < 0)
    {
        perror("send failed");
    }
    free(response);
    close(sock);
}

static void framec_set_response(framec_t *framec, http_response *response)
{
    framec->response = response;
}

static void framec_set_request(framec_t *framec, http_request *request)
{
    framec->request = request;
}

void framec_start()
{
    framec_t *framec = framec_create();
    pthread_setspecific(global_server->frame_key, framec);
    database_init();
}

void framec_terminate()
{
    framec_destroy();
}

static char *framec_handle_session(framec_t *framec)
{
    session_t *session = session_create();
    framec->session = session;

    char *id = hashmap_get(framec->request->cookies, "SESSIONID");
    if (id)
    {
        session->id = strdup(id);
    } else {
        session->id = NULL;
    }

    return session->id;
}

static void framec_set_session_cookie(framec_t *framec)
{
    if (framec->session->id)
    {
        http_response_set_cookie(framec->response, "SESSIONID", framec->session->id, "/", 3600);
    }
}

void framec_handle(int sock, char *buffer)
{
    framec_t *framec = get_framec();
    framec_set_request(framec, http_create_request());
    if (framec->request == NULL)
    {
        framec_set_response(framec, http_response_error(500, "Internal Server Error"));
        framec_send_response(sock, framec);
        return;
    }
    parser_parse_request(buffer, framec->request);
    if (framec->request->error)
    {
        framec_set_response(framec, http_response_error(400, "Bad Request"));
        framec_send_response(sock, framec);
        return;
    }
    framec_handle_session(framec);
    session_start(framec->session);


    pthread_mutex_lock(&global_server->lock);
    framec_set_response(framec, router_handle_request(global_server->router, framec->request));
    pthread_mutex_unlock(&global_server->lock);

    framec_set_session_cookie(framec);
    session_end(framec->session);

    framec_send_response(sock, framec);
}

void framec_response_set_header(char *key, char *value)
{
    framec_t *framec = get_framec();
    http_response_add_header(framec->response, key, value);
}

void framec_response_set_status(int status)
{
    framec_t *framec = get_framec();
    framec->response->status_code = status;
}

http_request *framec_get_request()
{
    return get_framec()->request;
}

char *framec_request(char *key, char *def)
{
    framec_t *framec = get_framec();
    char *value = http_request_get_param(framec->request, key);
    if (!value)
    {
        value = http_request_get_get_param(framec->request, key);
    }

    if (value) return value;
    return def;
}

void framec_session_set(char *key, char *value)
{
    framec_t *framec = get_framec();
    session_t *session = framec->session;

    session_set(session, key, value);
}

char *framec_session_get(char *key, char *def)
{
    framec_t *framec = get_framec();
    session_t *session = framec->session;

    char *value = session_get(session, key);
    if (!value)
    {
        return def;
    }
    return value;
}

const char *framec_env(char *key, char *def)
{
    framec_t *framec = get_framec();
    return config_get(framec->config, key, def);
}