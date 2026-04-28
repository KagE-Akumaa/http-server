# http-server

A modular HTTP/1.1 server built from scratch in modern C++, using raw POSIX socket APIs. Built to understand how HTTP actually works at the systems level — TCP behavior, partial reads, path security, binary I/O, and clean component design.

---

## Features

**Core**
- TCP server over POSIX sockets (`socket`, `bind`, `accept`, `read`/`write`)
- Thread-per-connection model — each client gets its own `std::thread`
- RAII socket management — no manual cleanup, no leaked file descriptors

**HTTP Parsing**
- Parses request line, headers, and body
- Handles `Content-Length` for body reads
- Correctly handles **partial TCP reads** — accumulates data across `read()` calls and detects `\r\n\r\n` on the full buffer

**Routing**
- `GET` and `POST` route registration
- Express-style lambda handlers: `router.get("/path", handler)`
- Clean separation between route matching and handler logic

**Static File Serving**
- Serves files from a configured root directory
- Prevents **directory traversal attacks** (`../../etc/passwd`)
- Prevents **prefix attacks** (`/public_malicious`)
- Guards against **symlink escapes**
- Canonical path validation via `std::filesystem::weakly_canonical`

**MIME Resolution**
- File extension → `Content-Type` mapping
- Fallback to `application/octet-stream` for unknown types

**Binary-Safe I/O**
- Files read and transmitted as raw bytes — images, fonts, and binary assets served correctly

---

## Project Structure

```
.
├── include/
│   ├── Http_Server.hpp       # Server lifecycle, socket setup, connection dispatch
│   ├── Router.hpp            # Route registration and matching
│   ├── Http_Parser.hpp       # Incremental HTTP request parser
│   ├── StaticFileHandler.hpp # Secure static file serving
│   ├── MimeResolver.hpp      # Extension → Content-Type mapping
│   ├── FileReader.hpp        # Binary-safe file I/O
│   ├── Http_Request.hpp      # Parsed request struct
│   └── Http_Response.hpp     # Response builder
│
├── src/
│   ├── Http_Server.cpp
│   ├── Router.cpp
│   ├── Http_Parser.cpp
│   ├── StaticFileHandler.cpp
│   ├── MimeResolver.cpp
│   └── FileReader.cpp
│
├── assets/                   # Static content served by the server
│   ├── images/
│   │   └── anime-bg.png
│   └── 404.html              # Custom not-found page
└── main.cpp
```

---

## Architecture

```
Client → HTTP_SERVER → HTTP_Parser → Router → Handler
                                            ↓
                               StaticFileHandler / API Handler
                                            ↓
                                      HTTP_Response
```

Each incoming connection is dispatched to a new thread. The parser accumulates raw bytes until `\r\n\r\n` is detected, then parses headers and reads the body if `Content-Length` is set. The router matches the method and path, and invokes the registered handler.

---

## Building

```bash
git clone https://github.com/KagE-Akumaa/http-server
cd http-server
make
./server
```

Server starts on `http://localhost:8989`.

> Requires a C++17-capable compiler, `make`, and a POSIX-compliant OS (Linux/macOS).

---

## Usage

### Defining Routes

```cpp
router.get("/", [](const Http_Request& req, Http_Response& res) {
    res.status(200).json(R"({"users":[{"id":1,"name":"Mukul"}]})");
});

router.post("/", [](const Http_Request& req, Http_Response& res) {
    res.status(201).json(R"({"message":"created"})");
});
```

### Static Files

Files under `assets/` are served automatically:

```
GET /images/anime-bg.png  →  assets/images/anime-bg.png
```

---

## Testing

```bash
# Basic GET
curl http://localhost:8989/

# POST with JSON body
curl -X POST http://localhost:8989 \
     -H "Content-Type: application/json" \
     -d '{"name":"test","email":"test@example.com"}'

# Static file
curl http://localhost:8989/images/anime-bg.png

# 404
curl http://localhost:8989/unknown

# Directory traversal attempt (should be rejected)
curl http://localhost:8989/../../../etc/passwd
```

---

## Security

This project explicitly addresses common HTTP server vulnerabilities:

| Attack | Mitigation |
|---|---|
| Directory traversal (`../../etc/passwd`) | `weakly_canonical` + root containment check |
| Prefix attack (`/public_malicious`) | Component-wise path comparison, not string prefix |
| Symlink escape | Canonical resolution follows symlinks before validation |

String-based path matching is intentionally avoided — path components are resolved and compared, not searched for substrings.

---

## Design Decisions

**Why thread-per-connection?**  
Simple to reason about and sufficient for a learning/demo context. The tradeoff is that it doesn't scale past a few hundred concurrent connections — a thread pool or `epoll`-based event loop would be the next step.

**Why RAII for sockets?**  
File descriptors are resources. Wrapping them in a class with a destructor ensures they're closed on any exit path — including exceptions — without needing explicit cleanup in every code path.

**Why `weakly_canonical` over `canonical`?**  
`std::filesystem::canonical` throws if the path doesn't exist. `weakly_canonical` resolves symlinks for existing components only, which is the right behavior when serving files that may or may not exist (produces a clean 404 rather than an exception).

---

## Roadmap

- [ ] Keep-alive connections (persistent `recv_buffer` per connection, multi-request loop)
- [ ] Thread pool (bounded concurrency, task queue)
- [ ] `epoll`-based async I/O
- [ ] Incremental streaming parser (state machine — eliminates double-buffering)
- [ ] Full HTTP/1.1 compliance (`Transfer-Encoding: chunked`, etc.)
- [ ] Structured logging
- [ ] Unit tests (GoogleTest or Catch2)

---

## Key Takeaways

- TCP delivers a **stream, not messages** — partial reads are the norm, not an edge case
- String prefix matching for paths is a security vulnerability, not a shortcut
- Binary vs text mode matters — opening a file in text mode corrupts images
- RAII is not optional in systems code — any manual `close()` is a potential leak
- Separation of concerns (parser / router / handler) makes each component independently testable

---

## License

MIT
