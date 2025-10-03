📌 HTTP Server Learning Path (MVP-Focused)
Week 1: Basic Sockets & Networking

Goal: Understand TCP/IP sockets, client/server communication, reading/writing data.

APUE Topics:

Ch. 3: File I/O → reading/writing to file descriptors

Ch. 16: Sockets → TCP server/client basics

Other Resources:

Beej’s Guide: TCP sockets, bind(), listen(), accept(), connect().

Minimal HTTP server tutorials in C++ (look for “single-threaded server”).

Exercises:

Create a server that accepts one client, reads raw bytes, prints to console.

Send a hardcoded HTTP response like "HTTP/1.1 200 OK\n\nHello" back.

Week 2: Request Parsing & Headers

Goal: Parse raw HTTP requests into a structured HttpRequest object.

APUE Topics:

Ch. 3: File I/O → handling reading loops for sockets

Ch. 10: Signals → optional, if you want Ctrl+C to close server gracefully

Other Resources:

HTTP RFC 7230 → understand request line, headers, body.

Blogs/tutorials on “parsing HTTP requests in C++”.

Exercises:

Parse method, path, and HTTP version from the request line.

Parse headers into a std::unordered_map.

Read body based on Content-Length header for POST requests.

Week 3: Response Handling & Routing

Goal: Build HttpResponse object and implement route handling (like Express).

APUE Topics:

Ch. 3/4: File descriptors & I/O → sending responses over sockets

Other Resources:

HTTP RFC 7231 → status codes, headers, content-length.

Minimal routing tutorials in C++ HTTP server projects.

Exercises:

Implement HttpResponse struct (status code, headers, body).

Implement sending response over client socket.

Create a simple routing table: map /hello → handler function.

Week 4: End-to-End Server & Testing

Goal: Connect all pieces for a fully working MVP.

APUE Topics:

Ch. 11: Threads → optional for later multi-client support

Ch. 16: Sockets → advanced options like SO_REUSEADDR

Other Resources:

Blogs or GitHub repos of minimal C++ HTTP servers for reference.

curl or httpie for testing your server.

Exercises:

Accept multiple sequential requests from clients.

Test GET and POST routes with JSON payloads.

(Optional) Add basic logging: print request and response info to console.est with curl/browser.
