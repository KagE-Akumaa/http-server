# What is a socket?

Sockets are a way to enable inter-process communication between programs running on a server, or between programs running on separate servers. Communication between servers relies on network sockets, which use the Internet Protocol (IP) to encapsulate and handle sending and receiving data.

Network sockets on both clients and servers are referred to by their socket address. An address is a unique combination of a transport protocol like the Transmission Control Protocol (TCP) or User Datagram Protocol (UDP), an IP address, and a port number.

# Everything in UNIX is a file?

When Unix programs do any sort of I/O, they do it by reading or writing to a file descriptor. A file descriptor is simply an integer associated with an open file. But (and here’s the catch), that file can be a network connection, a FIFO, a pipe, a terminal, a real on-the-disk file, or just about anything else. Everything in Unix is a file! So when you want to communicate with another program over the Internet you’re gonna do it through a file descriptor.

# How to get the file descriptor for network communication?

You make call to the socket() system routine. It returns the socket descriptor, and you can communicate through it using send(), recv() socket calls.

# If it's a file desciptor why can't we use read() and write()?

The short answer you can use read() and write() but send() and recv() gives you much greater control over your data transmission.

# Two types of Internet Sockets.

1. Stream Sockets.
2. Datagram Sockets.

# What is a Stream Socket?

Stream sockets are connection oriented, which means that packets sent to and received from a network socket are delivered by the host operating system in order for processing by an application. Network based stream sockets typically use the Transmission Control Protocol (TCP) to encapsulate and transmit data over a network interface.

TCP is designed to be a reliable network protocol that relies on a stateful connection. Data that is sent by a program using a TCP-based stream socket will be successfully received by a remote system (assuming there are no routing, firewall, or other connectivity issues). TCP packets can arrive on a physical network interface in any order. In the event that packets arrive out of order, the network adapter and host operating system will ensure that they are reassembled in the correct sequence for processing by an application.

# What is Telnet?

Telnet is a network protocol (and also a command-line tool) that lets you:

Connect to remote machines over TCP

Send raw text-based commands

And receive responses directly over the terminal

A manual client that lets you talk to a TCP server line by line.
You type → Telnet sends → Server responds → You see the raw output.

# 🌐 What is TCP?

TCP stands for Transmission Control Protocol — it’s one of the core protocols of the Internet and is responsible for reliable, ordered, and error-checked delivery of data between computers.

# 📦 In Simple Terms:

TCP is like a phone call:
Once connected, both sides can talk back and forth reliably — if one side misses something, it asks for it again.

# 🛠 How it Works (Simplified):

Connection Setup (3-way handshake):

Client: SYN

Server: SYN-ACK

Client: ACK

Data Transfer

Connection Teardown

# 🔌 Example:

When you connect to a website:

Your browser makes a TCP connection to the server (usually port 80 or 443)

HTTP data (like GET /index.html) is sent over this TCP connection

The server responds, also over TCP

# ✅ In Simple Terms:

A stream socket is a socket that uses TCP as its transport protocol — which means it provides a reliable, ordered, and error-checked stream of data between two computers.

# 📦 What is a Datagram Socket?

A datagram socket is a socket that uses UDP (User Datagram Protocol) instead of TCP.

In C/C++ or any socket API, it's created using:

```cpp

int sock = socket(AF_INET, SOCK_DGRAM, 0);
```

Here, SOCK_DGRAM tells the OS:

“I want a UDP socket, not a stream-based (TCP) socket.”

# 📚 Key Characteristics of Datagram (UDP) Sockets

Property Description
Protocol Uses UDP
Connectionless No connection is established — each message is standalone
Unreliable No guarantees: packets may be lost, duplicated, or arrive out of order
Message-oriented Sends discrete packets — each sendto() matches one recvfrom() call
Faster No handshake, no retransmission = low latency
Lightweight Small header overhead (8 bytes vs TCP’s ~20 bytes)

# 📦 How it works

There’s no connect() needed (though you can use it for convenience).

You use sendto() and recvfrom() to send/receive data to/from IP:port pairs.

# How a packet is formed?

# 📦 What Is Data Encapsulation?

Encapsulation is the process of wrapping data with necessary protocol information (headers) at each layer of the OSI or TCP/IP model.

Each layer adds its own header (and sometimes a trailer) to the data coming from the layer above it.

# 🧱 TCP/IP Model (4 Layers):

```diff

+----------------------+
| Application Layer | ← Your app (HTTP, DNS, etc.)
+----------------------+
| Transport Layer | ← TCP or UDP
+----------------------+
| Network Layer | ← IP
+----------------------+
| Link Layer | ← Ethernet/Wi-Fi
+----------------------+
```

# 🧪 Example: Sending "Hello" over HTTP

Step-by-step encapsulation (say it's a TCP socket over Ethernet):

1.  Application Layer:
    You write "Hello" in your browser → HTTP wraps it in a request

         GET / HTTP/1.1
         Host: example.com

    ⬇️

2.  Transport Layer (TCP):
    TCP adds a TCP header (with port numbers, sequence, ACK, etc.)

Now it becomes:

     [TCP Header][HTTP Data]
     ⬇️

3. Network Layer (IP):
   IP adds an IP header (with source & destination IPs)

   [IP Header][TCP Header][HTTP Data]
   ⬇️

4. Link Layer (Ethernet):
   Ethernet adds Ethernet header + trailer

Final packet:

     [Ethernet Header][IP Header][TCP Header][HTTP Data][Ethernet Trailer]

# 📦 Full Example Structure (TCP over Ethernet):

Layer Header Length Example Info Included
Ethernet 14 bytes MAC addresses
IP 20 bytes Source/Dest IP, TTL, Protocol (TCP/UDP)
TCP 20 bytes (min) Ports, Seq/Ack numbers, flags
HTTP/Data Variable Your actual message (e.g., GET /)
Ethernet CRC 4 bytes Error-checking (trailer)
