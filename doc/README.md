# Memory Pool Library

## Overview

The Memory Pool Library is designed for Arduino and other embedded systems, providing efficient dynamic memory management with fixed-size segments. It minimizes memory fragmentation and is optimized for resource-constrained environments.

## Features

- Fixed-size segment allocation
- Support for multiple segment sizes
- Bit-mapped allocation tracking
- Debug statistics (with `MEMPOOL_DEBUG`)
- Type-safe template-based allocation

## Installation

1. Download the library ZIP from the repository.
2. In the Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library` and select the ZIP file.
3. The library is ready to use.

## File List

- `mempool.h`: Header file with class and structure definitions.
- `mempool.tpp`: Template implementation for type-safe allocation.
- `mempool.cpp`: Core implementation of the memory pool.
- `keywords.txt`: Syntax highlighting for Arduino IDE.
- `library.properties`: Library metadata.
- `examples/mempool_example/mempool_example.ino`: Example sketch.
- `README.md`: This file.
- `API.md`: Detailed API documentation.
- `README.adoc`: AsciiDoc version of the README.

## Usage

Check the `examples/mempool_example` folder for a sample sketch. The library supports both raw byte allocation and type-safe template allocation.

## License

MIT License. See the `LICENSE` file for details.