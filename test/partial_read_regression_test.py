#!/usr/bin/env python3
"""Regression tests for HTTP headers fragmented across TCP reads.

Build the server first, then run:
    python3 test/partial_read_regression_test.py ./build/http-server
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


def assert_no_response_yet(client: socket.socket) -> None:
    readable, _, _ = select.select([client], [], [], 0.2)
    if readable:
        raise AssertionError("server responded before the complete header boundary")


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
            raise AssertionError("server did not parse the complete request") from error
        if not part:
            raise AssertionError("connection closed before a complete response")
        data.extend(part)

    header_end = data.find(b"\r\n\r\n")
    length_header = header_value(bytes(data[:header_end]), "content-length")
    if length_header is None:
        raise AssertionError(f"response missing Content-Length: {bytes(data)!r}")
    content_length = int(length_header)
    body_start = header_end + 4
    while len(data) < body_start + content_length:
        try:
            part = client.recv(4096)
        except socket.timeout as error:
            raise AssertionError("server did not parse the complete request") from error
        if not part:
            raise AssertionError("connection closed before a complete body")
        data.extend(part)
    return bytes(data[: body_start + content_length])


def send_fragmented_request(chunks: list[bytes]) -> bytes:
    with socket.create_connection((HOST, PORT), timeout=2) as client:
        client.settimeout(2)
        client.sendall(chunks[0])
        assert_no_response_yet(client)
        for chunk in chunks[1:]:
            client.sendall(chunk)
        return read_http_response(client)


def assert_successful_response(response: bytes) -> None:
    if not response.startswith(b"HTTP/1.1 200 OK\r\n"):
        raise AssertionError(f"expected a 200 response, got: {response!r}")


def run_case(name: str, chunks: list[bytes]) -> None:
    assert_successful_response(send_fragmented_request(chunks))
    print(f"PASS: {name}")


def main() -> None:
    executable = Path(sys.argv[1] if len(sys.argv) == 2 else "build/http-server")
    if not executable.is_file():
        raise SystemExit(f"server executable not found: {executable}")

    process = subprocess.Popen(
        [str(executable)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )
    try:
        wait_for_server(process)
        run_case(
            "header value split across reads",
            [b"GET / HTTP/1.1\r\nHost: local", b"host\r\n\r\n"],
        )
        run_case(
            "header boundary split across reads",
            [b"GET / HTTP/1.1\r\nHost: localhost\r\n\r", b"\n"],
        )
    finally:
        process.terminate()
        try:
            process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()


if __name__ == "__main__":
    main()
