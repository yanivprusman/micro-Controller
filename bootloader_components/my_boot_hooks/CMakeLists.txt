idf_component_register(SRCS "hooks.c" 
)
# set(MY_MAIN_DIR "C:/101_coding/102_esp/myRemoteDevice/main/")
# set(MY_MAIN_DIR "${CMAKE_CURRENT_LIST_DIR}/../../main/")

set(COMPONENT_ADD_INCLUDEDIRS "." "C:/101_coding/102_esp/myRemoteDevice/main/")
message("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdf")
message(STATUS " - COMPONENT_ADD_INCLUDEDIRS: ${COMPONENT_ADD_INCLUDEDIRS}")
# set(COMPONENT_ADD_INCLUDEDIRS "." "${MY_MAIN_DIR}")
set(LINKER_SCRIPT "C:/101_coding/102_esp/myRemoteDevice/main/linker_script.ld")
# set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/your_linker_script.ld")

# target_link_libraries(${PROJECT_NAME} INTERFACE "-T C:/101_coding/102_esp/myRemoteDevice/main/linker_script.ld")
# We need to force GCC to integrate this static library into the
# bootloader link. Indeed, by default, as the hooks in the bootloader are weak,
# the linker would just ignore the symbols in the extra. (i.e. not strictly
# required)
# To do so, we need to define the symbol (function) `bootloader_hooks_include`
# within hooks.c source file.
