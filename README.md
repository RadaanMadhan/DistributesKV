# DistributesKV

This is my personal, EC2-deployable eventually-consistent replicated key–value store (V1).

Overview
--------
DistributesKV implements a small replicated in-memory key–value store using consistent hashing and quorum-based reads/writes. The codebase is intentionally minimal to iterate quickly; it includes a local RPC shim for integration testing and a simple line-based TCP API for basic manual interaction.

This repository is my personal project and is intended to be deployed to EC2 instances in the future — the README below focuses on practical build, test, and simple deployment notes rather than an educational tutorial.

What is implemented (V1)
- Request routing using virtual nodes and consistent hashing (`src/router`)
- Thread-safe in-memory storage engine with versioned values (`src/storage`)
- Replication coordinator implementing quorum reads/writes (`src/replication`)
- In-process RPC shim for end-to-end tests (`src/replication/irpc.*`)
- A small line-based TCP API server for PUT/GET requests (`src/api`)
- Unit & integration tests using GoogleTest (see `tests/`)

Build & Run
-----------
Recommended: build and run tests via CMake (project already includes tests):

```bash
mkdir -p build && cd build
cmake ..
make
ctest --output-on-failure
```

There is also a top-level `Makefile` that compiles all sources into `build/distributes_kv`.

API (line-based TCP)
---------------------
The server implements a minimal, human-friendly protocol for testing and quick checks:

- PUT: `PUT <key> <value>\n` → replies `OK\n` or `ERR <message>\n`
- GET: `GET <key>\n` → replies `VALUE <value> <timestamp> <node_id>\n` or `NOT_FOUND\n`

Example using `nc` (netcat):

```bash
# send a value
printf "PUT mykey hello\n" | nc 127.0.0.1 15000

# read a value
printf "GET mykey\n" | nc 127.0.0.1 15000
```

Testing
-------
- Unit tests live in `tests/` and use GoogleTest. The CMake setup collects unit and integration tests.
- `tests/test_api_server.cpp` runs an in-process server and exercises PUT/GET.

EC2 Deployment Notes (short)
---------------------------
These are quick, practical pointers for later deployment to EC2:

- Packaging: build on an identical AMI or use a reproducible CI job that produces a tarball or deb package.
- Systemd: create a `systemd` unit for the server binary to manage lifecycle and restarts.
- Security Groups: open the application port (e.g., 15000) only to expected clients.
- Logging & Metrics: add structured logs and expose basic metrics (HTTP endpoint or push to CloudWatch).
- Configuration: add a config file or env-vars for binding address, port, cluster membership, and quorum settings.
- Secrets/Keys: avoid embedding secrets — use IAM roles, AWS Parameter Store, or Secrets Manager for credentials.

Next Steps (suggested)
- Split the client into a separate binary with a simple CLI for easy manual testing.
- Add CI (GitHub Actions) to build and run tests on push/PR.
- Harden networking, introduce retries, and improve observability before running on EC2.

Files of interest
- `src/replication/replication_coordinator.{hpp,cpp}` — quorum logic and read-repair hooks
- `src/router/request_router.{hpp,cpp}` — consistent hashing and vnode ring
- `src/storage/storage_engine.{hpp,cpp}` — in-memory, versioned key storage
- `src/api/api.{hpp,cpp}` — minimal TCP server
- `tests/` — unit and integration tests

License & Contact
-----------------
This repository is my personal project. Use or adapt as you like; reach out if you want to collaborate.
