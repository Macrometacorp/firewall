#include <macrometa.h>
#include <curlwrapper.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCKED_API_URL "/_api/ipbl/blocked/"
#define REGISTER_ID_API_URL "/_api/ipbl/register"

int is_blacklisted_mm(const char* ip, const char* app_key, const char* endpoint, const char* ipblc_id) 
{
    char url[MAX_URL] = "";
    strcat(url, endpoint);
    strcat(url, BLOCKED_API_URL);
    strncat(url, ip, 16);

    struct curl_request req = create_curl_request(url);
    strncpy(req.authorization, app_key, MAX_BODY);
    strncpy(req.x_ipblc_id, ipblc_id, MAX_BODY);

    struct curl_response *resp = execute_curl_request(&req);
    if (!resp) 
    {
        //if we don't know - consider it open
        return 0;
    }

    printf("[%d] Executing %s, resp: %s \n", resp->code, req.url, resp->body);

    int blacklisted = 0;

    if (resp->code == 200) 
    {
        blacklisted = 1;
    }

    free(resp);

    return blacklisted;
}

int register_machine(const char* app_key, const char* endpoint, char ipblc_id[MAX_BODY])
{
    char url[MAX_URL] = "";
    strcat(url, endpoint);
    strcat(url, REGISTER_ID_API_URL);

    char machine_id[255] = {'\0'};
    FILE *fp = fopen("/etc/machine-id", "r");
    if (fp == NULL)
    {
        return -1;
    }
    char* res = fgets(machine_id, 255, (FILE*)fp);
    fclose(fp);

    struct curl_request req = create_curl_request(url);
    strncpy(req.authorization, app_key, MAX_BODY);
    strncpy(req.data, machine_id, MAX_BODY);

    struct curl_response *resp = execute_curl_request(&req);
    if (!resp)
    {
        return -3;
    }

    printf("[%d] Executing %s, resp: %s \n", resp->code, req.url, resp->body);

    int blacklisted = 0;

    if (resp->code == 200) 
    {
        strcpy(ipblc_id, resp->body);
    }
    else
    {
        return -2;
    }

    free(resp);

    return 0;
}
