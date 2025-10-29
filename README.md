# AstraNet

AstraNet is a teaching-friendly C++ networking framework that demonstrates how to build an event-driven reactor from first principles. The codebase is intentionally compact and well-documented, making it a great starting point for understanding multiplexed I/O, timers, and connection management on Linux.

## Highlights

- **Learning-Oriented Layout** – Minimal dependencies and clearly separated modules (`base/` and `reactor/`) make it easy to trace execution paths.
- **Event-Driven Architecture** – Uses efficient I/O multiplexing (epoll) and non-blocking sockets to scale to many concurrent connections.
- **Thread Awareness** – Utilities for thread identification and coordination simplify working in multi-threaded environments.
- **Built-in Timers and Buffers** – Timer queues and buffer management utilities cover the foundations needed for robust network services.

## Installation

1. Ensure you have a modern C++ toolchain (GCC 7+/Clang 6+) and CMake 3.10 or newer installed.
2. Clone the repository and enter it:
   ```bash
   git clone <your-fork-url> && cd AstraNet
   ```
3. Configure and build the sample executable (which links all framework components):
   ```bash
   cmake -S test -B build
   cmake --build build
   ```

The build artifacts will be placed in `build/`. You can replace `test13.cc` inside `test/` with your own experiments or point the CMake configuration at different test sources.

## Usage

After building, run the bundled demo to verify the setup:

```bash
./build/test
```

This executable is wired to whatever source file is referenced by `TEST_SOURCES` inside `test/CMakeLists.txt`. By default it builds `test13.cc`, which is a small TCP client that connects to `localhost:8000` and prints the server's responses.

### Trying other samples from `test/`

The `test/` directory contains a collection of progressively more involved examples. To try a different sample:

1. Replace the `file(GLOB TEST_SOURCES ...)` line in `test/CMakeLists.txt` with the file you want to compile, for example:
   ```cmake
   file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test9.cc)
   ```
2. Reconfigure and rebuild:
   ```bash
   cmake -S test -B build
   cmake --build build
   ```
3. Run `./build/test` again.

The most useful samples include:

| Source file     | Highlights |
|-----------------|------------|
| `test3.cc`      | Shows how to register a timerfd with `Channel` and stop the loop on timeout. |
| `test4.cc`      | Demonstrates `runAfter` and `runEvery` scheduling helpers on `EventLoop`. |
| `test5.cc`      | Walks through `runInLoop`, `queueInLoop`, and callback ordering. |
| `test6.cc`      | Spins up an `EventLoopThread` and executes callbacks from another thread. |
| `test7.cc`      | Uses `Acceptor` to listen on multiple ports and greet new connections. |
| `test8.cc`      | Implements a simple TCP server that logs incoming messages. |
| `test9.cc`      | Extends the server into an echo service with configurable thread count. |
| `test10.cc`     | Sends pre-built payloads to connected clients and then shuts down. |
| `test12.cc`     | Illustrates the lower-level `Connector` helper used by `TcpClient`. |
| `test13.cc`     | Default TCP client example that connects to `127.0.0.1:8000`. |

More specialized recipes live under `test/recipies/`, such as the threading demonstrations in `test/recipies/thread/`. They can be copied into a standalone `.cc` file and compiled through the same mechanism if you need a starting point for experiments.

To create your own application, add a new `.cc` file under `test/`, point `TEST_SOURCES` at it, and rebuild. All framework headers under `base/` and `reactor/` are already exposed to the test build target, so you can focus on composing the pieces you need.

### Quick start: build an echo server

The snippet below mirrors `test9.cc` and shows the typical control flow for an echo service. Copy it into `test/echo_server.cc` (or any name you like) and set `TEST_SOURCES` accordingly:

```c++
#include "TcpServer.h"
#include "EventLoop.h"

using namespace mylib;  // Core AstraNet types live in the mylib namespace.

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp)
{
    conn->send(buf->retrieveAllAsString());  // bounce the payload back to the client
}

int main()
{
    EventLoop loop;
    InetAddress listenAddr(9981);  // listen on TCP port 9981

    TcpServer server(&loop, listenAddr);
    server.setMessageCallback(onMessage);
    server.setThreadNum(2);  // optional: scale the I/O worker pool
    server.start();

    loop.loop();  // block until the process is terminated
}
```

Reconfigure and rebuild:

```bash
cmake -S test -B build
cmake --build build
```

You can now run `./build/test` and connect with any TCP client:

```bash
./build/test &             # start the echo server in the background
nc 127.0.0.1 9981          # connect using netcat
hello AstraNet!            # type a message
hello AstraNet!            # the server echoes it back
```

The same structure works for other patterns: define your callbacks, construct a `TcpServer` or `TcpClient`, then hand control to `EventLoop::loop()`.

## Directory Layout

- `base/` – Thread utilities, logging, and timestamp helpers that underpin the rest of the framework.
- `reactor/` – Core reactor implementation, including `EventLoop`, `Channel`, `Epoller`, and TCP abstractions.
- `test/` – Self-contained samples and recipes that exercise different parts of the framework.

## License

This project is provided for educational purposes. Feel free to adapt it to suit your own experiments and learning goals.
