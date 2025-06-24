# Memory Pool Library API Documentation

## Overview

The Memory Pool Library provides a dynamic memory management system for Arduino, allowing efficient allocation and deallocation of memory in fixed-size segments.

## Classes

### `segment`

A structure to define a memory segment.

- **Fields**:
  - `uint16_t count`: Number of cells in the segment.
  - `uint8_t size`: Size of each cell in bytes.
- **Constructor**:
  - `segment(uint16_t c, uint16_t s)`: Initializes a segment with `c` cells of size `s`.

### `mempool`

The main class for memory pool management.

- **Constructor**:
  - `mempool()`: Creates an uninitialized memory pool.
- **Destructor**:
  - `~mempool()`: Frees all allocated resources.

## Methods

### `bool begin(segment* segs, uint8_t count)`

Initializes the memory pool with an array of segments.

- **Parameters**:
  - `segs`: Array of `segment` structures.
  - `count`: Number of segments (max 32).
- **Returns**: `true` on success, `false` on failure (e.g., invalid segments or already initialized).

### `uint8_t* alloc(uint16_t size)`

Allocates a memory block of the specified size.

- **Parameters**:
  - `size`: Size of the memory block in bytes.
- **Returns**: Pointer to the allocated memory, or `nullptr` if allocation fails.

### `template <typename T> T* alloc(uint8_t count)`

Allocates memory for an array of type `T`.

- **Parameters**:
  - `count`: Number of elements of type `T`.
- **Returns**: Pointer to the allocated memory, or `nullptr` if allocation fails.

### `void release(uint8_t* ptr)`

Releases a previously allocated memory block.

- **Parameters**:
  - `ptr`: Pointer to the memory block to release.

### `template <typename T> void release(T* ptr)`

Releases a previously allocated memory block of type `T`.

- **Parameters**:
  - `ptr`: Pointer to the memory block to release.

### `void print_buffer(uint8_t f = 2)`

Prints the buffer content to Serial.

- **Parameters**:
  - `f`: Format of the output (default: 2 for binary).

### `void print_segment_lookup(uint8_t f = 10)`

Prints the segment lookup table to Serial.

- **Parameters**:
  - `f`: Format of the output (default: 10 for decimal).

### `void print_stats()`

Prints memory pool statistics to Serial, including total and failed allocations and per-segment usage.

## Constants

- `SEGMENT_STEP`: Step size for segment allocation (default: 4).
- `SEGMENT_LOG2`: Log2 of segment step (default: 2).

## Notes

- The library uses a bit-mapped allocation system to track memory usage.
- Debug features (enabled with `MEMPOOL_DEBUG`) provide allocation statistics.
- Maximum segment count is 32, and maximum cell size is 64 bytes.