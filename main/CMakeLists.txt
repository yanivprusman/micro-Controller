
idf_component_register(
    SRCS "requestTaskBundle.c" "time_sync.c" "root_ca_cert.c" "exapmleCom2.c" "exampleCom.c" "wifi.c" "ota.c" "consoleCommands.c" "consoleRegister.c" "ble.c" "strip.c" "nvsFunctions.c" "myRemoteDevice.c"  
    # SRCS "../../109_smallProject/main/smallProject.c"
    INCLUDE_DIRS "."  
#     EMBED_TXTFILES ${project_dir}/certs/serverCert.pem
#     ${project_dir}/certs/serverKey.pem
#     ${project_dir}/certs/serverCert2.pem
    REQUIRES  esp_event nvs_flash bt esp_eth esp_http_client esp_http_server json 
    console app_update esp_https_ota mbedtls esp_https_server bootloader_support
    REQUIRES bootloader bootloader_support 
    target_link_libraries(${COMPONENT_LIB} INTERFACE "-T ${CMAKE_CURRENT_SOURCE_DIR}/linker_script.ld") 
)

# target_include_directories(${COMPONENT_LIB} PRIVATE 
# ${IDF_PATH}/components/bootloader_support/include)    
# idf_build_get_property(target IDF_TARGET)
# set(target_folder "${target}")

# # message(STATUS, "Target FOLDER:${target_folder}\n")

# set(scripts "${IDF_PATH}/components/bootloader/subproject/main/ld/${target_folder}/bootloader.ld"
#             "${IDF_PATH}/components/bootloader/subproject/main/ld/${target_folder}/bootloader.rom.ld")

# target_linker_script(${COMPONENT_LIB} INTERFACE "${scripts}")
# set(include_dirs "include" "bootloader_flash/include"
# "private_include")
