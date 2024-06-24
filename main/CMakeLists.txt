
idf_component_register(SRCS "time.c" "exapmleCom2.c" "exampleCom.c" "wifi.c" "ota.c" "consoleCommands.c" "consoleRegister.c" "ble.c" "strip.c" "nvsFunctions.c" "myRemoteDevice.c"  
                    INCLUDE_DIRS "."  
                    EMBED_TXTFILES ${project_dir}/certs/serverCert.pem
                    ${project_dir}/certs/serverKey.pem
                    ${project_dir}/certs/serverCert2.pem
                    )

