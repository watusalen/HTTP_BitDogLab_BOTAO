# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# CMake mínimo e padrões
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Caminho do SDK do Pico
set(PICO_SDK_PATH "")

# Definição da placa
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o SDK
include(pico_sdk_import.cmake)

# Projeto
project(picow_http_client C CXX ASM)
pico_sdk_init()

# Biblioteca auxiliar com funções HTTP
pico_add_library(example_lwip_http_util NOFLAG)
target_sources(example_lwip_http_util INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/example_http_client_util.c
)
target_include_directories(example_lwip_http_util INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/config
)
pico_mirrored_target_link_libraries(example_lwip_http_util INTERFACE
    pico_lwip_http
    pico_lwip_mbedtls
    pico_mbedtls
)

# === Aplicação principal ===
add_executable(picow_http_client
    ${CMAKE_CURRENT_LIST_DIR}/src/picow_http_client.c
)
target_compile_definitions(picow_http_client PRIVATE
    WIFI_SSID="" # Defina o SSID da rede Wi-Fi
    WIFI_PASSWORD="" # Defina a senha da rede Wi-Fi
)
pico_set_program_name(picow_http_client "picow_http_client")
pico_set_program_version(picow_http_client "0.1")

pico_enable_stdio_uart(picow_http_client 0)
pico_enable_stdio_usb(picow_http_client 1)

target_include_directories(picow_http_client PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/config
)

target_link_libraries(picow_http_client
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    example_lwip_http_util
    hardware_adc
)
pico_add_extra_outputs(picow_http_client)

# === Aplicação de verificação HTTPS com certificado ===
add_executable(picow_http_client_verify
    ${CMAKE_CURRENT_LIST_DIR}/src/picow_http_verify.c
)
target_compile_definitions(picow_http_client_verify PRIVATE
    WIFI_SSID="" # Defina o SSID da rede Wi-Fi
    WIFI_PASSWORD="" # Defina a senha da rede Wi-Fi
    ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED
)
target_include_directories(picow_http_client_verify PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/config
)
target_link_libraries(picow_http_client_verify
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    example_lwip_http_util
)
pico_add_extra_outputs(picow_http_client_verify)

# Supressão de warnings
set_source_files_properties(
    ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
    PROPERTIES COMPILE_OPTIONS "-Wno-unused-result"
)
