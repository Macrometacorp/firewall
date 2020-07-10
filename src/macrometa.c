#include <macrometa.h>
#include <curlwrapper.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int is_blacklisted_mm(const char* ip, const char* app_key, const char* api_url) 
{
    char url[MAX_URL] = {'\0'};
    strcpy(url, api_url);
    strncat(url, ip, 16);

    struct curl_request req = create_curl_request(url);
    strncpy(req.authorization, app_key, MAX_BODY);

    struct curl_response *resp = execute_curl_request(&req);
    printf("[%d] Executing %s, resp: %s \n", resp->code, req.url, resp->body);

    int blacklisted = 0;

    if (resp->code == 200 && strstr(resp->body, "\"value\":\"blacklisted\"") != NULL) 
    {
        blacklisted = 1;
    }

    free(resp);

    return blacklisted;
}

