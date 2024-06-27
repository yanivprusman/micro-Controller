
#include <stdio.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"

#include <esp_event.h>
#include <esp_system.h>
#include <esp_https_server.h>

// Define client certificate
extern  uint8_t ClientCert_pem_start[] asm("_binary_serverCert_pem_start");
extern  uint8_t ClientCert_pem_end[]   asm("_binary_serverCert_pem_end");

// Define server certificates
extern  unsigned char ServerCert_pem_start[] asm("_binary_serverCert_pem_start");
extern  unsigned char ServerCert_pem_end[] asm("_binary_serverCert_pem_end");
extern  unsigned char ServerKey_pem_start[] asm("_binary_serverKey_pem_start");
extern  unsigned char ServerKey_pem_end[] asm("_binary_serverKey_pem_end");

// WiFi
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

// Server
static esp_err_t server_get_handler(httpd_req_t *req)
{
    const char resp[] = "Server GET Response .................";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t server_post_handler(httpd_req_t *req)
{
    char content[100];
    size_t recv_size = MIN(req->content_len, sizeof(content));
    int ret = httpd_req_recv(req, content, recv_size);

    // If no data is send the error will be:
    // W (88470) httpd_uri: httpd_uri: uri handler execution failed
    printf("\nServer POST content: %s\n", content);

    if (ret <= 0)
    { /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    /* Send a simple response */
    const char resp[] = "Server POST Response .................";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t server_uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = server_get_handler
};

static const httpd_uri_t server_uri_post = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = server_post_handler
};

static httpd_handle_t start_webserver(void)
{
    // Start the httpd server
    printf("Starting server\n");
    
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    httpd_handle_t server = NULL;
    // ServerCert_pem_start = ClientCert_pem_start;
    // ServerCert_pem_end = ClientCert_pem_end;
    config.cacert_pem = ServerCert_pem_start;
    config.cacert_len = ServerCert_pem_end - ServerCert_pem_start;
    config.prvtkey_pem = ServerKey_pem_start;
    config.prvtkey_len = ServerKey_pem_end - ServerKey_pem_start;

    esp_err_t err = httpd_ssl_start(&server, &config);
    if (ESP_OK != err)
    {
        printf("Error starting server!%s\n",esp_err_to_name(err));
        return NULL;
    }

    // Set URI handlers
    printf("Registering URI handlers\n");
    httpd_register_uri_handler(server, &server_uri_get);
    httpd_register_uri_handler(server, &server_uri_post);
    return server;
}

// Client
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("Client HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

static void client_post_rest_function(char*uri)
{
    esp_http_client_config_t config_post = {
        // .url = "https://firestore.googleapis.com/v1/projects/sims-iot-devices/databases/(default)/documents/my_collection",
        .url = uri,
        .method = HTTP_METHOD_POST,
        .cert_pem = (const char *)ClientCert_pem_start,
        .event_handler = client_event_get_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "{\"fields\":{\"Memory\":{\"doubleValue\":\"2\"},\"Name\":{\"stringValue\":\"Data from the client\"}}}";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

void exampleCom2(char*uri)
{
    esp_log_level_set("*", ESP_LOG_DEBUG);
    printf("Start server:\n\n");
    start_webserver();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("Start client:\n\n");
    client_post_rest_function(uri);
}