# NetForge 2.0

NetForge is a Linux-first **C17 network diagnostics and host-inspection toolkit** built to demonstrate systems programming: POSIX sockets, non-blocking I/O, pthread concurrency, Linux `epoll`, DNS, HTTP, TLS/OpenSSL, CIDR discovery, structured output, and latency statistics.

> Use NetForge only on systems and networks you own or are explicitly authorized to test.

## Highlights

- Two TCP engines: bounded **pthread + poll** and event-driven **Linux epoll**
- IPv4/IPv6 resolution and TCP probing
- IPv4 CIDR host discovery
- DNS A/AAAA resolution
- Raw HTTP metadata inspection
- TLS handshake/certificate inspection with OpenSSL
- Passive service/banner identification on greeting-based protocols
- Combined `inspect` host report
- JSON and CSV scan output
- Latency min/average/p50/p95/max statistics
- Interface enumeration
- Strict compiler warnings, CMake, CTest, sanitizer-tested code

## Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake libssl-dev
```

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

## Examples

```bash
./build/netforge interfaces
./build/netforge dns example.com
./build/netforge probe 127.0.0.1 8080
./build/netforge http 127.0.0.1 8080
./build/netforge tls example.com 443
./build/netforge scan 127.0.0.1 1-1024 --open-only
./build/netforge scan 127.0.0.1 1-1024 --engine epoll --open-only
./build/netforge scan 127.0.0.1 1-1024 --engine epoll --csv
./build/netforge --json scan 127.0.0.1 22,80,443 --engine epoll
./build/netforge discover 192.168.1.0/24 443
./build/netforge inspect example.com
```

For scan testing, prefer loopback (`127.0.0.1`), your own VM, or networks where you have explicit authorization.

## Architecture

```text
                         NetForge CLI
                              |
                         core/app.c
                              |
       +----------------------+----------------------+
       |                      |                      |
    Commands                Output                 Config
       |                 text/json/csv        timeout/concurrency
       |
 +-----+---------+------------+-------------+
 |               |            |             |
scan          discover      inspect         tls
 |               |            |             |
 +-------+-------+------------+-------------+
         |
    Network layer
 +-------+---------+----------+----------+
 |                 |          |          |
TCP engines       DNS        HTTP       TLS/OpenSSL
 |                                      
 +-- pthread/poll
 +-- Linux epoll
```

### pthread/poll backend

A bounded worker pool claims ports from a mutex-protected queue. Each worker performs a non-blocking connection and uses `poll()` plus `SO_ERROR` to determine connection state.

### epoll backend

The Linux backend maintains a bounded number of non-blocking connection attempts in one event loop. `epoll` reports socket readiness while NetForge tracks per-connection deadlines and latency. This provides an architectural comparison between thread-based and event-driven concurrency.

### TLS inspection

The TLS module performs a normal client handshake using OpenSSL and reports negotiated protocol/cipher, certificate subject/issuer, validity dates, verification result, and handshake latency.

### Statistics

Scan results calculate minimum, average, median (p50), p95, and maximum observed connection latency along with open/closed/error counts.

## Project scope

NetForge is a diagnostics/learning project. It intentionally does not implement exploit execution, credential attacks, stealth/evasion, or vulnerability exploitation.

## Suggested future work

- SQLite scan history and diffing
- ncurses/notcurses TUI
- Config-file support
- Unit tests for parsing/statistics modules
- Optional topology model for owned lab networks
- Performance benchmark harness comparing pthread/poll and epoll

## Resume description

> Built NetForge, a modular Linux network diagnostics toolkit in C17 featuring POSIX sockets, non-blocking TCP, pthread/poll and epoll concurrency backends, IPv4/IPv6 resolution, CIDR discovery, HTTP/TLS inspection, OpenSSL certificate parsing, JSON/CSV reporting, and latency percentile analysis.
