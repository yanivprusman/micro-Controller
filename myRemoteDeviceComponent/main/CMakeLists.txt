file(TO_CMAKE_PATH "$ENV{MRD_INCLUDE_PATH}" MRD_INCLUDE_PATH)
file(TO_CMAKE_PATH "$ENV{MRD_LIB}" MRD_LIB)
idf_component_register(SRCS "main.c"
    INCLUDE_DIRS ""
    ${MRD_INCLUDE_PATH}
    #     "../components/myRemoteDevice")
)
# add_prebuilt_library(myRemoteDevice "libmyRemoteDevice.a"
#     REQUIRES spi_flash app_update log espressif__led_strip esp_https_server )
# target_link_libraries(${COMPONENT_LIB} PRIVATE myRemoteDevice)
# # REQUIRES app_update esp_https_ota app_update esp_http_client console driver json bt nvs_flash esp_https_server 
# idf_component_register(
#     SRCS "main.c"
#     INCLUDE_DIRS ". "
#     ${MRD_INCLUDE_PATH}
#     REQUIRES espressif__led_strip app_update esp_https_ota app_update esp_http_client console driver json bt nvs_flash esp_https_server 
#     # REQUIRES myRemoteDeviceComponent/main/New folder/idf_component.ymlapp_update esp_https_ota app_update esp_http_client console driver json bt nvs_flash esp_https_server 
#     )
     

# add_prebuilt_library(myRemoteDevice "${MRD_LIB}"
# PRIV_REQUIRES spi_flash app_update log console espressif__led_strip)
# target_link_libraries(${COMPONENT_LIB} PRIVATE myRemoteDevice)
