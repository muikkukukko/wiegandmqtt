cmake_minimum_required(VERSION 3.19)

# Read settings from JSON file
file(READ "${CMAKE_CURRENT_LIST_DIR}/settings.json" settings_json)

string(JSON PICO_BOARD GET "${settings_json}"
       settings board)
string(JSON WIFI_SSID GET "${settings_json}"
       settings wifi ssid)
string(JSON WIFI_PASSWORD GET "${settings_json}"
       settings wifi password)
string(JSON MQTT_SERVER_HOST GET "${settings_json}"
       settings mqtt host)
string(JSON MQTT_SERVER_PORT GET "${settings_json}"
       settings mqtt port)

# Read certificates and client key to variables
execute_process(
    COMMAND 
    python 
    ${CMAKE_CURRENT_LIST_DIR}/certificate_helper/to_header_file_format.py 
    ${CMAKE_CURRENT_LIST_DIR}/certificate_helper/generated/client.key
    OUTPUT_VARIABLE CLIENT_KEY
)

execute_process(
    COMMAND 
    python 
    ${CMAKE_CURRENT_LIST_DIR}/certificate_helper/to_header_file_format.py 
    ${CMAKE_CURRENT_LIST_DIR}/certificate_helper/generated/client.crt
    OUTPUT_VARIABLE CLIENT_CERT
)

execute_process(
    COMMAND 
    python 
    ${CMAKE_CURRENT_LIST_DIR}/certificate_helper/to_header_file_format.py 
    ${CMAKE_CURRENT_LIST_DIR}/certificate_helper/generated/ca.crt
    OUTPUT_VARIABLE CA_CERT
)

#Check that certificates are set
if((NOT CLIENT_KEY) OR (NOT CLIENT_CERT) OR (NOT CA_CERT))
    message(FATAL_ERROR "Certificates not set. Build failed!")
endif()

#Check that wifi settings are set
if((NOT WIFI_SSID) OR (NOT WIFI_PASSWORD))
    message(FATAL_ERROR "Wifi settings not set. Build failed!")
endif()

# Construct MQTT header file from template, settings and certificates
configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/mqttsettings.h.in"
    "${CMAKE_CURRENT_LIST_DIR}/mqttsettings.h"
)

# Set env board selection 
set(ENV{PICO_BOARD} ${PICO_BOARD})


# Pull in SDK (must be before project)
include(pico_sdk_import.cmake) 

project(wiegandmqtt C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

if (NOT PICO_CYW43_SUPPORTED) # set by BOARD=pico-w
    message(FATAL_ERROR "CYW43 Wifi chip not supported")
endif()


add_executable(wiegandmqtt)
add_subdirectory(src)

# Enable usb output
pico_enable_stdio_usb(wiegandmqtt 1)
pico_enable_stdio_uart(wiegandmqtt 0)

# Generate PIO header
pico_generate_pio_header(wiegandmqtt ${CMAKE_CURRENT_LIST_DIR}/src/wiegand.pio)

target_compile_definitions(wiegandmqtt PRIVATE
WIFI_SSID=\"${WIFI_SSID}\"
WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
TEST_TCP_SERVER_IP=\"${TEST_TCP_SERVER_IP}\"
NO_SYS=1
CYW43_SPI_PIO_PREFERRED_PIO=1
)

target_include_directories(wiegandmqtt PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)

target_link_libraries(wiegandmqtt
    pico_cyw43_arch_lwip_poll
    pico_stdlib
    pico_lwip_mbedtls
    pico_mbedtls
    pico_lwip_mqtt
    hardware_pio
) 

pico_add_extra_outputs(wiegandmqtt)

