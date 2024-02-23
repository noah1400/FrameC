#include <parser.h>

// parse http request using the mpc parser
parsed_object *parse(char *input)
{

    /* Define all parsers within mpca_lang */
    mpc_parser_t *HttpMethod = mpc_new("http_method");
    mpc_parser_t *RequestURI = mpc_new("request_uri");
    mpc_parser_t *HttpVersion = mpc_new("http_version");
    mpc_parser_t *RequestLine = mpc_new("request_line");
    mpc_parser_t *HeaderField = mpc_new("header_field");
    mpc_parser_t *Headers = mpc_new("headers");
    mpc_parser_t *MessageBody = mpc_new("message_body");
    mpc_parser_t *Request = mpc_new("request");

    mpca_lang(MPCA_LANG_DEFAULT,
              " http_method    : (\"GET\" | \"POST\" | \"PUT\" | \"DELETE\" | \"HEAD\" | \"OPTIONS\" | \"CONNECT\" | \"TRACE\");"
              " request_uri    : /([^ ]+)/;"
              " http_version   : /HTTP\\/1\\.[0-1]/;"
              " request_line   : <http_method> <request_uri> <http_version>;"
              " header_field   : /[^:\\r\\n]+: [^\\r\\n]+/;"
              " headers        : (<header_field>)*;"
              " message_body   : /(.*(\n.*)*)?/;"
              " request        : /^/ <request_line> <headers> <message_body> /$/;",
              HttpMethod, RequestURI, HttpVersion, RequestLine, HeaderField, Headers, MessageBody, Request, NULL);

    mpc_result_t r;
    if (mpc_parse("input", input, Request, &r))
    {
        mpc_ast_t *a = (mpc_ast_t *)r.output;

        // traverse the AST 
        mpc_ast_t *request_line = a->children[1];
        mpc_ast_t *http_method = request_line->children[0];
        mpc_ast_t *request_uri = request_line->children[1];
        mpc_ast_t *http_version = request_line->children[2];

        mpc_ast_t *headers = a->children[2];
        mpc_ast_t *message_body = a->children[3];

        // output each part of the request
        printf("Method: %s\n", http_method->contents);
        printf("URI: %s\n", request_uri->contents);
        printf("Version: %s\n", http_version->contents);
        printf("Headers: \n");
        for (int i = 0; i < headers->children_num; i++)
        {
            printf("\t%s\n", headers->children[i]->contents);
        }
        printf("Body: %s\n", message_body->contents);       

        mpc_ast_delete(a);
        mpc_cleanup(8, HttpMethod, RequestURI, HttpVersion, RequestLine, HeaderField, Headers, MessageBody, Request);
        return NULL; // Return a properly filled parsed_object instead
    }
    else
    {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
        mpc_cleanup(8, HttpMethod, RequestURI, HttpVersion, RequestLine, HeaderField, Headers, MessageBody, Request);
        return NULL;
    }
}