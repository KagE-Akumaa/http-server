#!/usr/bin/env python3
"""Integration test: two HTTP/1.1 requests on one TCP connection.

Build the server first, then run:
    python3 test/keep_alive_integration_test.py ./build/http-server
"""

import select
import socket
import subprocess
import sys
import time
from pathlib import Path


HOST = "127.0.0.1"
PORT = 8989


def wait_for_server(process: subprocess.Popen[bytes]) -> None:
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise RuntimeError("server exited before accepting connections")
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
            if probe.connect_ex((HOST, PORT)) == 0:
                return
        time.sleep(0.05)
    raise RuntimeError("server did not start listening within five seconds")


def header_value(headers: bytes, name: str) -> str | None:
    text = headers.decode("latin1")
    target = name.lower()
    for line in text.split("\r\n")[1:]:
        key, _, value = line.partition(":")
        if key.lower() == target:
            return value.strip()
    return None


def read_http_response(client: socket.socket) -> bytes:
    data = bytearray()
    while b"\r\n\r\n" not in data:
        try:
            part = client.recv(4096)
        except socket.timeout as error:
            raise AssertionError("timed out waiting for response headers") from error
        if not part:
            raise AssertionError("connection closed before a complete response")
        data.extend(part)

    header_end = data.find(b"\r\n\r\n")
    headers = bytes(data[:header_end])
    body_start = header_end + 4
    length_header = header_value(headers, "content-length")
    if length_header is None:
        raise AssertionError(f"response missing Content-Length: {bytes(data)!r}")
    content_length = int(length_header)

    while len(data) < body_start + content_length:
        try:
            part = client.recv(4096)
        except socket.timeout as error:
            raise AssertionError("timed out waiting for response body") from error
        if not part:
            raise AssertionError("connection closed before a complete body")
        data.extend(part)

    return bytes(data[: body_start + content_length])


def assert_response(response: bytes, status_prefix: bytes, connection: str) -> None:
    if not response.startswith(status_prefix):
        raise AssertionError(f"expected {status_prefix!r}, got: {response!r}")
    header_end = response.find(b"\r\n\r\n")
    actual = header_value(response[:header_end], "connection")
    if actual is None or actual.lower() != connection:
        raise AssertionError(
            f"expected Connection: {connection}, got {actual!r} in {response!r}"
        )


def test_two_requests_one_connection() -> None:
    with socket.create_connection((HOST, PORT), timeout=2) as client:
        client.settimeout(2)
        client.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
        first = read_http_response(client)
        assert_response(first, b"HTTP/1.1 200 OK\r\n", "keep-alive")

        readable, _, _ = select.select([client], [], [], 0.2)
        if readable:
            peek = client.recv(4096, socket.MSG_PEEK)
            if peek == b"":
                raise AssertionError(
                    "server closed the socket after the first keep-alive response"
                )

        client.sendall(
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        second = read_http_response(client)
        assert_response(second, b"HTTP/1.1 200 OK\r\n", "close")

        leftover = client.recv(4096)
        if leftover:
            raise AssertionError(
                f"expected the server to close after Connection: close, got {leftover!r}"
            )


def test_pipelined_requests() -> None:
    with socket.create_connection((HOST, PORT), timeout=2) as client:
        client.settimeout(2)
        client.sendall(
            b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
            b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        )
        first = read_http_response(client)
        second = read_http_response(client)
        assert_response(first, b"HTTP/1.1 200 OK\r\n", "keep-alive")
        assert_response(second, b"HTTP/1.1 200 OK\r\n", "close")


def main() -> None:
    executable = Path(sys.argv[1] if len(sys.argv) == 2 else "build/http-server")
    if not executable.is_file():
        raise SystemExit(f"server executable not found: {executable}")

    process = subprocess.Popen(
        [str(executable)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )
    try:
        wait_for_server(process)
        test_two_requests_one_connection()
        print("PASS: two sequential requests on one TCP connection")
        test_pipelined_requests()
        print("PASS: two pipelined requests on one TCP connection")
    finally:
        process.terminate()
        try:
            process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()


if __name__ == "__main__":
    main()
