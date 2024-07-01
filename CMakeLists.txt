# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
# list(APPEND EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)
cmake_minimum_required(VERSION 3.16)
# set(MAIN_COMPONENT_DIR "../109_smallProject/main")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(myRemoteDevice)
# message( "HI\n")
# set(COMPONENTS main esptool_py)
message( "HI\n")
# target_link_libraries(${COMPONENT_LIB} INTERFACE "-T ${CMAKE_CURRENT_SOURCE_DIR}/linker_script.ld") 
# set(EXTRA_LINKER_SCRIPT linker_script.ld)
# set(EXTRA_LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker_script.ld)
