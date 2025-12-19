# High-Performance Multi-Threaded HTTP Server

A lightweight, high-throughput Web Server built from scratch in **C++17** using the **Windows Sockets API (Winsock2)**.

Unlike typical web frameworks, this project implements the core networking layer manually to handle raw TCP connections, HTTP parsing, and concurrent client management using a custom **Thread Pool**.

## Key Features

* **Custom Thread Pool:** Implemented a pre-allocated pool of worker threads to handle concurrent connections efficiently, avoiding the overhead of creating and destroying threads per request.
* **Raw Socket Programming:** Built directly on `winsock2` without external networking libraries (like Boost.Asio) to demonstrate low-level systems knowledge.
* **Thread Safety:** Utilizes `std::mutex`, `std::unique_lock`, and `std::condition_variable` to manage the task queue safely across multiple threads.
* **Modern Build System:** Fully integrated with **CMake** for cross-platform build management.
* **HTTP Protocol:** Supports `GET` requests, serves static files (HTML, CSS, Images), and handles 404/500 errors.

## Architecture

```text
[Client] -> [Load Balancer / Socket Listener]
                     |
            [Job Queue (Thread Safe)]
                     |
       -----------------------------
       |             |             |
 [Worker Thread] [Worker Thread] [Worker Thread]
       |             |             |
    [Parse HTTP]  [Read File]   [Send Response]