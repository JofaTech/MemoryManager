# Custom Memory Manager in C++

## Overview

This project implements a custom memory manager in C++ designed to handle memory allocation and deallocation efficiently within a console application.

The memory manager is responsible for:

- Initializing and tracking memory usage.
- Allocating and deallocating memory blocks on demand.
- Maintaining a list of free memory holes and merging adjacent holes to reduce fragmentation.
- Writing a log of its state upon closing for debugging and verification purposes.

## Features

- Dynamic memory management mimicking operating system behavior.
- Integration with a console application that performs basic unit testing to verify correctness.
- Manual testing recommended for memory leaks and overall stability.

## Structure

The memory manager uses standard OS-layer functions and APIs to provide reliable memory operations while ensuring compatibility with console programs.

## Usage

- Compile the project using a C++ compiler supporting C++11 or higher.
- Run the console application to test allocation and deallocation scenarios.
- Review the generated logs to understand memory state changes over time.

## Notes

- While basic unit tests are included, it is important to perform additional memory leak detection using tools like Valgrind or similar.
- Proper merging of free memory holes is implemented to optimize memory usage.

---

Feel free to explore the code and contribute improvements or extensions.
