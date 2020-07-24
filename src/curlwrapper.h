#pragma once

#define MAX_URL 1024
#define MAX_BODY 1024

struct curl_request {
    char url[MAX_URL];
    char authorization[MAX_BODY]; 
    char x_ipblc_id[MAX_URL];
    char data[MAX_BODY];
};

struct curl_response {
    int code;
    char body[MAX_BODY];
};

struct curl_request create_curl_request(const char* url);

struct curl_response* execute_curl_request(struct curl_request* req);

