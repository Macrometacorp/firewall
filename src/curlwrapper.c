#include "curlwrapper.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

size_t write_function( void *ptr, size_t size, size_t nmemb, void *userdata)
{
    if(nmemb > 0)
    {
        char* buffer = (char*)userdata;

        if (strlen(buffer) + nmemb < MAX_BODY) {
            strncat(buffer, (char*) ptr, nmemb);
        }
    }
    return nmemb;
}


CURL* create_conversation(struct curl_request *req) 
{
    CURL *curl_handle = curl_easy_init();
    if(!curl_handle)
        return NULL;

    curl_easy_setopt(curl_handle, CURLOPT_URL, req->url);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_function);

    curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 3L);

    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 60);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "gzip");

    return curl_handle;
}

int parse_response_code(const char *headers) {
    const char* start = strchr(headers, ' ');
    if (start == NULL || strlen(start) < 5) {
        return 0;
    }

    char codeStr[4] = {'\0'};
    strncpy(codeStr,start + 1, 3);
    
    return atoi(codeStr);
}

struct curl_response* execute_curl_request(struct curl_request* req) 
{
    char error_buffer[CURL_ERROR_SIZE] = {'\0'};
    char headers_buffer[MAX_BODY] = {'\0'};
    CURL* curl_handle = create_conversation(req);
    if (curl_handle == NULL) {
        return NULL;
    }

    struct curl_response *resp = malloc(sizeof(struct curl_response));
    for(size_t i = 0; i < strlen(resp->body); i++)
    {
        resp->body[i] = '\0';
    }

    // Now set up all of the curl options
    //LOG_TRACE(CURL_PROVIDER, "Request: " << _url);
    
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, headers_buffer);
    curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, resp->body);

    if (strlen(req->data) > 0)
    {
        curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, req->data);
    } 
    else 
    {
        curl_easy_setopt(curl_handle, CURLOPT_POST, 0);
    }

    if (strlen(req->authorization) > 0)
    {
        struct curl_slist *headers=NULL;
        char header[MAX_BODY] = {'\0'};
        strcat(header, "X-API-KEY: ");
        strncat(header, req->authorization, MAX_BODY - strlen(header));
        headers = curl_slist_append(headers, header);

        if (strlen(req->x_ipblc_id) > 0)
        {
            char header[MAX_BODY] = {'\0'};
            strcat(header, "X-IPBLC-ID: ");
            strncat(header, req->x_ipblc_id, MAX_BODY - strlen(header));
            headers = curl_slist_append(headers, header);
        }
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl_handle);
    if (res != 0) {
        printf("[%d] Curl error with url %s, err: %s\n", res, req->url, curl_easy_strerror(res));
        return NULL;
    }
    resp->code = parse_response_code(headers_buffer);

    curl_easy_cleanup(curl_handle);

    return resp;
}

void zero_init(char* str, size_t len) 
{
    for(size_t i = 0; i < len; i++)
    {
        str[i] = '\0';
    }
}

struct curl_request create_curl_request(const char* url) 
{
    struct curl_request req;
    zero_init(req.url, sizeof(req.url));
    zero_init(req.data, sizeof(req.data));
    zero_init(req.authorization, sizeof(req.authorization));
    zero_init(req.x_ipblc_id, sizeof(req.x_ipblc_id));

    strcpy(req.url, url);

    return req;
}
