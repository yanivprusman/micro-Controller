#ifndef _OATA_
#define _OATA_

#include "myRemoteDevice.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_https_ota.h"
// #include "requestTaskBundle.h"
#include "esp_crt_bundle.h"

void ota_task(void *pvParameter);

#endif // _OATA_