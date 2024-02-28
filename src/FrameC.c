#include <framec.h>

static framec_t *framec_create();
static void framec_destroy();
static void framec_send_response(int sock, framec_t *framec);
static void framec_set_response(framec_t *framec, http_response *response);
static void framec_set_request(framec_t *framec, http_request *request);


static framec_t *framec_create()
{
    framec_t *framec = (framec_t *)malloc(sizeof(framec_t));
    if (framec == NULL)
    {
        perror("Could not allocate memory for framec_t");
        framec_terminate();
        exit(1);
    }
    return framec;
}

static void framec_destroy()
{
    framec_t *framec = (framec_t *)pthread_getspecific(global_server->frame_key);
    if (framec->request)
    {
        http_free_request(framec->request);
    }
    if (framec->response)
    {
        http_free_response(framec->response);
    }
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
}

void framec_terminate()
{
    framec_destroy();
}

void framec_handle(int sock, char *buffer)
{
    framec_t *framec = (framec_t *)pthread_getspecific(global_server->frame_key);
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
    pthread_mutex_lock(&global_server->lock);
    framec_set_response(framec, router_handle_request(global_server->router, framec->request));
    pthread_mutex_unlock(&global_server->lock);
    framec_send_response(sock, framec);
}

void framec_response_set_header(char *key, char *value)
{
    framec_t *framec = (framec_t *)pthread_getspecific(global_server->frame_key);
    http_response_add_header(framec->response, key, value);
}

void framec_response_set_status(int status)
{
    framec_t *framec = (framec_t *)pthread_getspecific(global_server->frame_key);
    framec->response->status_code = status;
}