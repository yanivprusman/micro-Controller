#include "requestTaskBundle.h"
// extern const uint8_t cStart[] asm("_binary_serverCert_pem_start");
// extern const uint8_t cEnd[]   asm("_binary_serverCert_pem_end");
// extern const uint8_t kStart[] asm("_binary_serverKey_pem_start");
// extern const uint8_t kEnd[]   asm("_binary_serverKey_pem_end");
// extern const uint8_t cStart2[] asm("_binary_serverCert2_pem_start");
// extern const uint8_t cEnd2[]   asm("_binary_serverCert2_pem_end");


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
    printf("host: %s\n",host);
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

void https_request_task_bundle(char*url)
{
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
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
        free(httpRequest);
        vTaskDelete(NULL);
    }
    
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
        for (int i = 0; i < len; i++) {
            putchar(buf[i]);
        }
    } while (1);
    esp_tls_conn_destroy(tls);
    free(httpRequest);
    vTaskDelete(NULL);
}
void requestTaskBundle(char*url){
    xTaskCreate(&https_request_task_bundle, "https_request_task", 8192, url, 5, NULL);

}