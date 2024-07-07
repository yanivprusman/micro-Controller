cmake_minimum_required(VERSION 3.16)

# Since we're only interested in the prebuilt library, trim
# the build
# set(COMPONENTS  main myRemoteDevice esptool_py console led_strip )
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(myRemoteDevice)
