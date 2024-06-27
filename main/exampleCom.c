#include "examplecom.h"
// #define URI "https://esp.ya-niv.com/myRemoteDevicePage"
// #define URI "https://example.com"
#include "esp_netif.h"
#include "esp_tls.h"
static char * str=NULL;
extern const uint8_t cacert_pem_start[] asm("_binary_serverCert_pem_start");
extern const uint8_t cacert_pem_end[]   asm("_binary_serverCert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            printf("HTTP_EVENT_ERROR\n");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            printf("HTTP_EVENT_ON_CONNECTED\n");
            break;
        case HTTP_EVENT_HEADER_SENT:
            printf("HTTP_EVENT_HEADER_SENT\n");
            break;
        case HTTP_EVENT_ON_HEADER:
            printf("HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            printf("HTTP_EVENT_ON_DATA, len=%d\n", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            printf("HTTP_EVENT_ON_FINISH\n");
            break;
        case HTTP_EVENT_DISCONNECTED:
            printf("HTTP_EVENT_DISCONNECTED\n");
            break;
        default:
    }
    return ESP_OK;
}


void http_get_task(void *pvParameters)
{
    char* uri = (char*)pvParameters;
    esp_err_t err = NULL;
    const char *cacert_pem = (char*)cacert_pem_start;
    esp_tls_cfg_t tls_cfg = {
        .cacert_pem_buf  = cacert_pem_start,
        .cacert_pem_bytes = cacert_pem_end - cacert_pem_start,
        // .clientcert_pem_buf = client_cert_pem_start,
        // .clientcert_pem_bytes = sizeof(client_cert_pem_start),
        // .clientkey_pem_buf = client_key_pem_start,
        // .clientkey_pem_bytes = sizeof(client_key_pem_start),
    };
    esp_http_client_config_t config = { 
        .url = uri,
        .event_handler = _http_event_handler,
        .cert_pem = cacert_pem,
        .skip_cert_common_name_check = true, // Disable common name check
        .use_global_ca_store = false, // Disable CA store
        .cert_len = cacert_pem_end - cacert_pem_start,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        
        // .transport_ssl_cfg = &tls_cfg,
        // .skip_cert_pem_check = true, // Disable cert_pem check
    };    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (!client){
        vTaskDelete(NULL);
    }; 
    err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        printf("HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        printf("HTTP GET request failed: %s", esp_err_to_name(err));
    }

    // Cleanup
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void exampleCom(const char*uri)
{
    esp_err_t err;

    err = esp_netif_init();
    if (err != ESP_OK) {
        printf("Error esp_netif_init: %s\n", esp_err_to_name(err));
    };
    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        printf("Error esp_event_loop_create_default: %s\n", esp_err_to_name(err));
    };
    wifi();
    xTaskCreate(&http_get_task, "http_get_task", 8192, uri, 5, NULL);
}
