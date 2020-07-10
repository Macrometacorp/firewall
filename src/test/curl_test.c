#include <curlwrapper.h>
#include <macrometa.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TEST_URL "http://127.0.0.1:4046/bla\0"
#define MACROMETA_KEY "bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjEuNTkzNzczOTMyMzA4OTE4NmUrNiwiZXhwIjoxNTkzODE3MTMyLCJpc3MiOiJhcmFuZ29kYiIsInByZWZlcnJlZF91c2VybmFtZSI6InJvb3QiLCJ0ZW5hbnQiOiJzbGF2Y2hvX2dtYWlsLmNvbSJ9.4dQ1JZowwR2pvuQnu3zNAJR6lnnJVNYOzM9U2Rv51N0="

int main() {

    struct curl_request req = create_curl_request(TEST_URL);
    struct curl_response *resp = execute_curl_request(&req);
    printf("[%d] Executing %s, resp: %s \n", resp->code, req.url, resp->body);

    const char* ip = "127.0.0.1";
    if (is_blacklisted_mm(ip, MACROMETA_KEY) == 1) {
        printf("Is blacklisted: %s", ip);
    } else {
        printf("Is not blacklisted: %s", ip);
    }

    free(resp);

    return 0;
}
