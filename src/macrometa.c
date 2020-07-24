#include <macrometa.h>
#include <curlwrapper.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>

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

int read_mac_address(char *mac_address)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) { /* handle error*/ };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        return -1;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else { 
            /* handle error */ 
            printf("failed to read address of %s\n", it->ifr_name);
        }
    }

    unsigned char mac[6];

    if (success) {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    } else {
        return -1;
    }

    sprintf(mac_address, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return 0;
}

int read_machine_id(char *machine_id) 
{
    FILE *fp = fopen("/etc/machine-id", "r");
    if (fp == NULL)
    {
        return -1;
    }
    char* res = fgets(machine_id, 255, (FILE*)fp);
    fclose(fp);

    return 0;
}

int read_ipbl_id(char* machine_id) {
    FILE *fp = fopen("ipbl.id", "r");
    if (fp == NULL)
    {
        return -1;
    }
    char* res = fgets(machine_id, 255, (FILE*)fp);
    fclose(fp);

    return 0;
}

int save_ipbl_id(char* machine_id) {
    FILE *fp = fopen("ipbl.id", "w");
    if (fp == NULL)
    {
        return -1;
    }
    fputs(machine_id, (FILE*)fp);
    fclose(fp);

    return 0;
}

int register_machine(const char* app_key, const char* endpoint, char ipblc_id[MAX_BODY])
{
    char url[MAX_URL] = "";
    strcat(url, endpoint);
    strcat(url, REGISTER_ID_API_URL);

    char machine_id[255] = {'\0'};

    if (read_ipbl_id(ipblc_id) == 0) {
        //already registered
        return 0;
    }

    if (read_mac_address(machine_id) != 0)
    {
        printf("Failed to read mac address. Falling back to machine id.\n");
        if (read_machine_id(machine_id) != 0) {
            printf("Failed to read machine id as well.\n");
            return -4;
        }
    }

    printf("machine id : %s\n", machine_id);

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

        save_ipbl_id(ipblc_id);
    }
    else
    {
        return -2;
    }

    free(resp);

    return 0;
}
