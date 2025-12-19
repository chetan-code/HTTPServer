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
```

## Build Instructions

### Prerequisites

* **C++ Compiler:** MSVC (Visual Studio), G++, or Clang.
* **CMake:** Version 3.10 or higher.
* **Operating System:** Windows (Required for `winsock2`).

### Step-by-Step Build Guide

1.  **Clone the Repository**
    ```bash
    git clone [https://github.com/chetan-code/HTTPServer.git](https://github.com/chetan-code/HTTPServer.git)
    cd HTTPServer
    ```

2.  **Create a Build Directory**
    It is best practice to keep build files separate from source code.
    ```bash
    mkdir build
    cd build
    ```

3.  **Generate Build Files**
    Run CMake to verify your compiler and generate the Makefiles or Visual Studio solution.
    ```bash
    cmake ..
    ```

4.  **Compile the Project**
    This command compiles the source code and links the `ws2_32` library.
    ```bash
    cmake --build .
    ```

### Running the Server

**Important:** The server looks for the `www` folder in its current working directory. You must run the executable from the **project root**, not from inside the build folder.

1.  Navigate back to the project root:
    ```bash
    cd ..
    ```

2.  Run the executable:
    ```bash
    .\build\Debug\server.exe
    ```

3.  **Verify:**
    Open your browser and navigate to: `http://localhost:8080`