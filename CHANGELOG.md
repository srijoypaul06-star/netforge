# Changelog

## 2.0.0
- Added Linux `epoll` event-driven TCP scan backend.
- Added selectable `--engine threads|epoll` scanning.
- Added TLS protocol/cipher/certificate inspection with OpenSSL.
- Added CSV scan export.
- Added scan latency statistics: min, average, p50, p95, max.
- Improved CLI help and versioning.
- Validated Release and ASan/UBSan builds.

## 1.1.0
- Added combined host `inspect` command.
- Added CIDR discovery and service/banner observations.

## 1.0.0
- Initial modular C17 networking toolkit.
