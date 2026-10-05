cmake_minimum_required(VERSION 3.16)
project(netforge VERSION 2.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wformat=2)

include_directories(include)

set(NETFORGE_SOURCES
    src/main.c
    src/core/app.c
    src/core/config.c
    src/net/tcp.c
    src/net/epoll_scan.c
    src/net/dns.c
    src/net/http.c
    src/net/interfaces.c
    src/net/network.c
    src/net/tls.c
    src/cmd/scan.c
    src/cmd/discover.c
    src/cmd/probe.c
    src/cmd/dns_cmd.c
    src/cmd/http_cmd.c
    src/cmd/interfaces_cmd.c
    src/cmd/inspect.c
    src/cmd/tls_cmd.c
    src/utils/log.c
    src/utils/output.c
    src/utils/str.c
    src/utils/stats.c
)

add_executable(netforge ${NETFORGE_SOURCES})
find_package(OpenSSL REQUIRED)
target_link_libraries(netforge PRIVATE pthread OpenSSL::SSL OpenSSL::Crypto)

if (UNIX AND NOT APPLE)
    target_link_libraries(netforge PRIVATE resolv)
endif()

enable_testing()
add_test(NAME smoke COMMAND netforge --help)
add_test(NAME help COMMAND netforge --help)
add_test(NAME loopback_epoll COMMAND netforge scan 127.0.0.1 1-4 --engine epoll)
add_test(NAME loopback_threads COMMAND netforge scan 127.0.0.1 1-4 --engine threads)
install(TARGETS netforge RUNTIME DESTINATION bin)
