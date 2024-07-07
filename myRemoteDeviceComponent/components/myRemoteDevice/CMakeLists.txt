
idf_component_register(
    SRCS "requestTaskBundle.c" "time_sync.c" "root_ca_cert.c" "exapmleCom2.c" "exampleCom.c" "wifi.c" "ota.c" "consoleCommands.c" "consoleRegister.c" "ble.c" "strip.c" "nvsFunctions.c" "myRemoteDevice.c"  
    INCLUDE_DIRS "."  
    REQUIRES  esp_event nvs_flash bt esp_eth esp_http_client esp_http_server json 
    console app_update esp_https_ota mbedtls esp_https_server bootloader_support
    REQUIRES bootloader bootloader_support 
)
target_link_libraries(${COMPONENT_LIB} PUBLIC "-T ${CMAKE_CURRENT_SOURCE_DIR}/linker_script.ld")
add_custom_command(TARGET ${COMPONENT_LIB}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${COMPONENT_LIB}> ${CMAKE_SOURCE_DIR}/../main
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/myRemoteDevice.h ${CMAKE_SOURCE_DIR}/../main
    COMMENT "Copying built archive file and header to parent directory...")
