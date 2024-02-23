#include <http.h>

http_response *http_response_json(int status_code, char *json_data) {
    http_response *resp = http_create_response(status_code, http_response_status_message(status_code), json_data);
    http_response_add_header(resp, "Content-Type", "application/json");
    return resp;
}

http_response *http_response_text(int status_code, char *text) {
    http_response *resp = http_create_response(status_code, http_response_status_message(status_code), text);
    // No additional content type header needed since text/plain is default
    return resp;
}

http_response *http_response_redirect(char *path) {
    http_response *resp = http_create_response(301, http_response_status_message(301), "");
    http_response_add_header(resp, "Location", path);
    // Content-Type not defined in redirect responses
    // Leave it text/plain will be ignored
    return resp;
}

char *http_response_status_message(int status_code) {
    switch (status_code) {
        case 200:
            return "OK";
        case 301:
            return "Moved Permanently";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 500:
            return "Internal Server Error";
        default:
            return "Unknown Status";
    }
}

http_response *http_response_error(int status_code, char *error_message) {
    return http_response_text(status_code, error_message);
}

void http_response_set_cookie(http_response *resp, char *name, char *value, char *path, int max_age) {
    char *cookie = malloc(strlen(name) + strlen(value) + strlen(path) + 50);
    sprintf(cookie, "%s=%s; Path=%s; Max-Age=%d", name, value, path, max_age);
    http_response_add_header(resp, "Set-Cookie", cookie);
    free(cookie);
}