#include <http.h>

http_response *http_response_json(int status_code, char *json_data) {
    http_response *resp = http_create_response(status_code, "OK", json_data);
    http_response_add_header(resp, "Content-Type", "application/json");
    return resp;
}

http_response *http_response_text(int status_code, char *text) {
    http_response *resp = http_create_response(status_code, "OK", text);
    // No additional content type header needed since text/plain is default
    return resp;
}

http_response *http_response_redirect(char *path) {
    http_response *resp = http_create_response(301, "Moved Permanently", "");
    http_response_add_header(resp, "Location", path);
    // Content-Type not defined in redirect responses
    // Leave it text/plain will be ignored
    return resp;
}