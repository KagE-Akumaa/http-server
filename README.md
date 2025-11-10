# ⚡ HTTP Server in C++

A lightweight and educational HTTP server built from scratch in **C++**.  
This project demonstrates how to handle HTTP requests, parse URLs, and serve responses — without using any external web frameworks.
A fully functional HTTP server built from scratch in C++ to understand how web servers process requests, handle routes, and manage connections at a low level.

Currently supports GET and POST routes, with planned support for PATCH and DELETE in the upcoming version. The goal is to make it a minimal yet extendable HTTP backend framework demonstrating complete request parsing, URL utilities, and response handling.

---

## 🚀 Features

- Built completely in **C++**
- Custom **HTTP request parsing**
- Basic **URL encoding/decoding utilities**
- Lightweight **server core** using sockets
- Organized modular structure (`mainServer`, `HTTP_SERVER`, `URL_Util`)
- Easy compilation via `Makefile`

---

## 🧩 Project Structure

```
├── mainServer.cpp      # Entry point, starts the HTTP server
├── HTTP_SERVER.cpp     # Core server logic (socket creation, handling, parsing)
├── URL_Util.cpp        # Utility for URL encoding/decoding
├── Makefile            # Build automation
└── README.md           # Project documentation
```

---

## ⚙️ Build Instructions

### 🔧 Prerequisites
- C++ compiler (e.g., `g++`)
- Linux/Unix system (tested on Ubuntu)

### 🏗️ Build the Project
Clone the repository:
```bash
git clone https://github.com/KagE-Akumaa/http-server.git
cd http-server
```

Compile the source:
```bash
make
```

This will generate an executable named:
```
server
```

---

## ▶️ Run the Server

To start the server:
```bash
./server
```

By default, it will:
- Initialize a socket
- Listen for incoming HTTP requests
- Parse and respond based on implemented routes

---

## 🧹 Clean Build Files
```bash
make clean
```
This removes the compiled binary:
```
rm -f server
```

---

## 💡 Example Output
```
[INFO] Server started on port 8080
[INFO] Waiting for client connection...
[REQUEST] GET /?name=test HTTP/1.1
[RESPONSE] 200 OK
```

---

## 🧠 Future Improvements
- Add support for multiple routes (e.g., `/api`, `/status`)
- Add logging and configuration support

---

## 🧑‍💻 Author
**Mukul Jogi**  
Built as part of a personal backend learning journey in C++.

---

## 📜 License
This project is open-source under the **MIT License**.
