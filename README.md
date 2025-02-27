# Bench Loggers

This repository provides benchmarks for two popular logging libraries: **spdlog** and **quill**. The purpose of this project is to compare the performance of these loggers.

## Overview

The benchmarks measure the time required to log a fixed number of messages under different threading conditions. In each test, the following parameters are varied:
- **Messages:** Total number of log messages sent.
- **Threads:** Number of threads used during logging.
- **Time:** Measured in milliseconds (ms) for logging the messages.

The leftmost column in each table represents the number of messages sent, while the top row corresponds to the number of threads used. The cells contain the measured times in milliseconds.

## Benchmark Setup

The benchmarks were conducted using:
- **spdlog (synchronous mode)**
- **spdlog (asynchronous mode with default thread pool size)**
- **quill**

These tests help illustrate how each logger scales with increased message volume and multi-threading.

## Results

### spdlog – Synchronous Mode

| **Messages**   | **1 Thread (ms)** | **2 Threads (ms)** | **4 Threads (ms)** | **8 Threads (ms)** |
|----------------|-------------------|--------------------|--------------------|--------------------|
| 10,000         | 3                 | 12                 | 18                 | 45                 |
| 100,000        | 34                | 91                 | 200                | 484                |
| 1,000,000      | 297               | 886                | 2060               | 5013               |
| 10,000,000     | 3097              | 8683               | 20812              | 49149              |

### spdlog – Asynchronous Mode (Default Thread Pool Size)

| **Messages**   | **1 Thread (ms)** | **2 Threads (ms)** | **4 Threads (ms)** | **8 Threads (ms)** |
|----------------|-------------------|--------------------|--------------------|--------------------|
| 10,000         | 8                 | 108                | 20                 | 41                 |
| 100,000        | 57                | 169                | 561                | 2620               |
| 1,000,000      | 591               | 1813               | 5841               | 30058              |
| 10,000,000     | 5762              | 18004              | 62508              | 294538             |

### quill

| **Messages**   | **1 Thread (ms)** | **2 Threads (ms)** | **4 Threads (ms)** | **8 Threads (ms)** |
|----------------|-------------------|--------------------|--------------------|--------------------|
| 10,000         | 3                 | 4                  | 0                  | 8                  |
| 100,000        | 26                | 25                 | 34                 | 46                 |
| 1,000,000      | 198               | 204                | 285                | 514                |
| 10,000,000     | 3169              | 3362               | 4246               | 6696               |

## Analysis

In almost all scenerious quill logger is faster. The strangest thing is that spdlog in synchronous mode is faster than the asynchronous version. A bottleneck of async mode is a message queue implemented using locks. In addition, when the queue is full, an attempt to log results in the thread being locked until space becomes available in the queue. (There are alternative policy that overrides old messages in queue, but isnt valid for our case). On the other hand, quill message queue is lock-free and will dynamicly resize queue on demand.
