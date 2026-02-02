# DistributesKV

DistributesKV is a minimal, educational eventually-consistent replicated key–value store (V1).

This repository implements a simple in-memory storage node with:

- Consistent hashing-based request routing
- Quorum-based reads and writes (configurable R/W/N)
- In-process RPC shim for local testing
- Simple line-based TCP API for PUT/GET (useful for quick manual tests)
- Unit and integration tests (GoogleTest)

Status (V1)
- Core replication coordinator, request router, and local storage engine implemented.
- Tests covering storage, routing, thread pool, basic replication flows and an API integration smoke test.

Quick Start (build & test)

1. Create a build directory and run CMake (recommended for tests):

```bash
mkdir -p build && cd build
cmake ..
make
ctest --output-on-failure
```

2. The integration API test starts an in-process server and exercises PUT/GET (see tests/test_api_server.cpp).

Run the API server (manual):
- The project provides an executable when built with the provided top-level Makefile (`make` at repo root).
- Alternatively run the test binary `build/tests/test_api_server` which starts the server on a free port for the test.

API Protocol (line-based TCP)
- PUT: `PUT <key> <value>\n` → replies: `OK\n` or `ERR <msg>\n`
- GET: `GET <key>\n` → replies: `VALUE <value> <timestamp> <node_id>\n` or `NOT_FOUND\n`

Examples (using netcat)

```bash
# send a value
printf "PUT mykey hello\n" | nc 127.0.0.1 15000
# read a value
printf "GET mykey\n" | nc 127.0.0.1 15000
```

Notes & Internals
- Request routing: `src/router/request_router.{hpp,cpp}` builds a ring of virtual nodes.
- Storage: `src/storage/storage_engine.{hpp,cpp}` holds VersionedValue entries in-memory.
- Replication: `src/replication/replication_coordinator.{hpp,cpp}` performs parallel RPCs and enforces quorum logic.
- Local RPC shim: `src/replication/irpc.*` registers local `StorageEngine` instances for integration tests.

Next steps (suggestions)
- Split client into a separate executable and add a simple CLI.
- Add CI (GitHub Actions) to run `cmake` + `ctest` on push.
- Add more robust network handling, metrics, and configuration parsing.

License & contact
- This is an educational sample project. Adapt and extend as you like.
