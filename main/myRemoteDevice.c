#include "myRemoteDevice.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
char * TAG = "MyRemoteDevice";

uint8_t initializedCustomDataTrue[4]  = {0x0, 0x0, 0x0, 0x0};
const uint8_t initializedCustomDataFalse[4] = {0xDF, 0xAD, 0xBE, 0xEE};

variables_t 
    initialized = {
        "initialized",
        NULL,
        "var_1",
        INITIALIZED_FALSE_STRING},
    ssidName={
        "ssidName",
        NULL,
        "var_2",
        ""},
    ssidPassword={
        "ssidPassword",
        NULL,
        "var_3",
        ""},
    myRemoteDeviceName={
        "myRemoteDeviceName",
        NULL,
        "var_4",
        "MyRemoteDevice"},
    myRemoteDeviceID={
        "myRemoteDeviceID",
        NULL,
        "var_5",
        ""};
variables_t * variables[]={&initialized,&ssidName,&ssidPassword,&myRemoteDeviceName,&myRemoteDeviceID};

void printVariables(){
    for (int i = 0;i < NUM_VARIABLES;i++){
        if((variables[i]->value)&&(variables[i]->name)){
            printf("the value of %s is \"%s\"\n",variables[i]->name,variables[i]->value);
        }
    }
}

void fillVariablseFromJsonString(char* data){
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        printf("Error parsing JSON: %s", cJSON_GetErrorPtr());
        return;
    }
    for (int i = 0;i < NUM_VARIABLES;i++){
        cJSON *got = cJSON_GetObjectItemCaseSensitive(json,variables[i]->name);
        if (cJSON_IsString(got) && (got->valuestring != NULL)) {
            if (variables[i]->value) free(variables[i]->value);
            variables[i]->value = strdup(got->valuestring);
        }
    }
    cJSON_Delete(json);
}

void readVariablesFromNvs(){
    esp_err_t err;
    nvs_handle_t storage;
    err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        printf("Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        size_t required_size = 0;
        err = nvs_get_str(storage, variables[i]->nvsKey, NULL, &required_size);
        if (err != ESP_OK) {
            printf("Error getting size for %s: %s", variables[i]->name, esp_err_to_name(err));
            continue;
        }
        variables[i]->value = malloc(required_size);
        if (variables[i]->value == NULL) {
            printf("Error allocating memory for %s", variables[i]->name);
            continue;
        }
        err = nvs_get_str(storage, variables[i]->nvsKey, variables[i]->value, &required_size);
        if (err != ESP_OK) {
            printf("Error getting value for %s: %s", variables[i]->name, esp_err_to_name(err));
            free(variables[i]->value);
            variables[i]->value = NULL;
            continue;
        }
    }
    nvs_close(storage);
}
void writeVariablesToNvs(){
    nvs_handle_t storage;
    nvs_open("storage", NVS_READWRITE, &storage);
    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        if (variables[i]->value != NULL){
            nvs_set_str(storage, variables[i]->nvsKey, variables[i]->value);
        };
    }
    nvs_commit(storage);
    nvs_close(storage);
}

void writeDefaultValues() {
    esp_err_t err;
    nvs_handle_t storage;
    // printf("nvs_open\n");
    err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        printf("Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    // printf("num vars: %d\n", NUM_VARIABLES);

    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        err = nvs_set_str(storage, variables[i]->nvsKey, variables[i]->defaultValue);
        if (err != ESP_OK) {
            printf("Error setting %s: %s", variables[i]->name, esp_err_to_name(err));
        }
    }
    // printf("nvs_commit\n");

    err = nvs_commit(storage);
    if (err != ESP_OK) {
        printf("Error committing NVS: %s", esp_err_to_name(err));
    }

    nvs_close(storage);
    // printf("end write\n");

}

bool variablesAreEmpty(){
    bool empty = false;
    if (myRemoteDeviceID.value==NULL){
        empty = true;
        printf("variable myRemoteDeviceId is NULL\n");
    }
    return empty;
}
void app_main()
{
    esp_reset_reason_t r = esp_reset_reason();
    // app2();
    // return;
    printf("last reset reason: %d\n",r);
    esp_err_t err = nvs_flash_init();
    if(err !=ESP_OK){
        printf("error1: %s\n",esp_err_to_name(err));
    };    
    err = esp_netif_init();
    if(err !=ESP_OK){
        printf("error2: %s\n",esp_err_to_name(err));
    };
    err = esp_event_loop_create_default();
    if(err !=ESP_OK){
        printf("error3: %s\n",esp_err_to_name(err));
    };
    // wifi();
    example_connect();
    
    nimble();
    doTime();
    readVariablesFromNvs();
    if(variablesAreEmpty()){
        writeDefaultValues();        
    }
    char *argv[3];
    argv[2] = "http://example.com";
    doFunction1(3,argv);
    console();
    printf("do 1 \"https://example.com\"\n");
    // doFunction2(3,argv);
    // console();
    // printf("do 2 \"https://example.com\"\n");
}

#include "esp_netif.h"
#include "esp_tls.h"

extern const uint8_t cStart[] asm("_binary_serverCert_pem_start");
extern const uint8_t cEnd[]   asm("_binary_serverCert_pem_end");
extern const uint8_t kStart[] asm("_binary_serverKey_pem_start");
extern const uint8_t kEnd[]   asm("_binary_serverKey_pem_end");
extern const uint8_t cStart2[] asm("_binary_serverCert2_pem_start");
extern const uint8_t cEnd2[]   asm("_binary_serverCert2_pem_end");

#include"esp_https_server.h"
#define WEB_SERVER "www.example.com"
#define WEB_PORT "443"
#define WEB_URL ""
#include "esp_tls.h"

// static const char HTTPS_REQUEST[] =
//     "GET " WEB_URL " HTTP/1.1\r\n"
//     "Host: "WEB_SERVER"\r\n"
//     "User-Agent: esp-idf/1.0 esp32\r\n"
//     "\r\n";

#include "esp_crt_bundle.h"

void get_host_from_url(const char *url, char *host, size_t host_size) {
    const char *start;
    const char *end;
    size_t len;

    // Find the start of the host
    if ((start = strstr(url, "://")) != NULL) {
        start += 3; // Skip "://"
    } else {
        start = url; // URL doesn't contain "://", assume it's the beginning
    }

    // Find the end of the host
    if ((end = strchr(start, '/')) != NULL) {
        len = end - start;
    } else {
        len = strlen(start);
    }

    // Ensure the host buffer is large enough
    if (len >= host_size) {
        len = host_size - 1; // Truncate if necessary
    }

    // Copy the host part to the host buffer
    strncpy(host, start, len);
    host[len] = '\0'; // Null-terminate the string
}

char* construct_http_request(const char *url) {
    char host[256];
    get_host_from_url(url, host, sizeof(host));

    // Calculate the length of the final HTTP request string
    size_t request_size = strlen("GET ") + strlen(url) + strlen(" HTTP/1.1\r\n") +
                          strlen("Host: ") + strlen(host) + strlen("\r\n") +
                          strlen("User-Agent: esp-idf/1.0 esp32\r\n\r\n") + 1;

    // Allocate memory for the HTTP request string
    char *httpRequest = (char *)malloc(request_size);
    if (httpRequest == NULL) {
        printf("Failed to allocate memory for HTTP request\n");
        return NULL;
    }

    // Construct the HTTP request string
    snprintf(httpRequest, request_size,
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: esp-idf/1.0 esp32\r\n"
             "\r\n",
             url, host);

    return httpRequest;
}

void https_request_task_bundle(void *pvparameters)
{
    args_t *args = (args_t*)pvparameters;
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    char * url = args->argv[2];
    char buf[512];
    int ret, len;
    esp_tls_t *tls = esp_tls_init();
    if (!tls) {
        printf("Failed to allocate esp_tls handle!\n");
        vTaskDelete(NULL);
    }
    ret =esp_tls_conn_http_new_sync(url, &cfg, tls); 
    if (ret != 1){
        printf("Failed to establish TLS connection: %d\n",ret);
        esp_tls_conn_destroy(tls);
        vTaskDelete(NULL);
    }
    char *httpRequest = construct_http_request(url);
    if (httpRequest == NULL) {
        vTaskDelete(NULL);
    }
        // printf("%s", httpRequest);
        // free(httpRequest); // Don't forget to free the allocated memory

    size_t written_bytes = 0;
    do {
        ret = esp_tls_conn_write(tls, httpRequest + written_bytes, strlen(httpRequest) - written_bytes);
        if (ret >= 0) {
            printf("%d bytes written\n", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            printf("esp_tls_conn_write returned -0x%x\n", -ret);
            break;
        }
    } while (written_bytes < strlen(httpRequest));

    // Read HTTPS response
    printf("Reading HTTP response...\n");
    do {
        len = sizeof(buf) - 1;
        memset(buf, 0x00, sizeof(buf));
        ret = esp_tls_conn_read(tls, buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_READ || ret == ESP_TLS_ERR_SSL_WANT_WRITE) {
            continue;
        } else if (ret < 0) {
            printf("esp_tls_conn_read returned -0x%x\n", -ret);
            break;
        } else if (ret == 0) {
            printf(TAG, "Connection closed\n");
            break;
        }

        len = ret;
        printf("%d bytes read\n", len);
        // Print response directly to stdout as it is read
        for (int i = 0; i < len; i++) {
            putchar(buf[i]);
        }
    } while (1);
    esp_tls_conn_destroy(tls);
    vTaskDelete(NULL);
}

int doFunction1(int argc, char **argv){
    if (argc != 3){
        printf("usage: do 1 <uri>\n");
        return -1;
    }
    args_t args = {argc,argv};
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    printf("creating task https_request_task: %s\n",argv[2]);
    xTaskCreate(&https_request_task_bundle, "https_request_task", 8192, &args, 5, NULL);
    return 0;
};
int doFunction2(int argc, char **argv){
    args_t args = {argc,argv};
    if (args.argc != 3){
        printf("usage: do 2 <uri>\n");
        return -1;
    }
    return 0;
}
void app2() {
}
