
idf_component_register(SRCS "requestTaskBundle.c" "time_sync.c" "root_ca_cert.c" "exapmleCom2.c" "exampleCom.c" "wifi.c" "ota.c" "consoleCommands.c" "consoleRegister.c" "ble.c" "strip.c" "nvsFunctions.c" "myRemoteDevice.c"  
    INCLUDE_DIRS "."  
    EMBED_TXTFILES ${project_dir}/certs/serverCert.pem
    ${project_dir}/certs/serverKey.pem
    ${project_dir}/certs/serverCert2.pem
    REQUIRES  esp_event nvs_flash bt esp_eth esp_http_client esp_http_server json 
    console app_update esp_https_ota mbedtls esp_https_server bootloader_support
    REQUIRES bootloader_support
    )

    target_include_directories(${COMPONENT_LIB} PRIVATE 
    ${IDF_PATH}/components/bootloader_support/include)    

