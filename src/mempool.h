// mempool.h
#pragma once
#include <stdint.h>

#define MAX_SEGMENT_SIZE 256

/**
 * @brief Structure to define a memory segment with count and size.
 */
struct segment {
  /**
   * @brief Constructor for segment.
   * @param c Number of cells in the segment.
   * @param s Size of each cell in the segment.
   */
  segment(uint16_t c, uint16_t s) : count(c), size(s) {}
  uint16_t count;  ///< Number of cells in the segment
  uint8_t size;    ///< Size of each cell
};

/**
 * @brief Memory pool class for dynamic memory management on Arduino.
 */
class mempool {
 public:
  /**
   * @brief Default constructor for mempool.
   */
  mempool();

  /**
   * @brief Destructor for mempool, cleans up allocated resources.
   */
  ~mempool();

  /**
   * @brief Frees all dynamically allocated memory and resets the pool.
   */
  void clean();

  /**
   * @brief Initializes the memory pool with given segments.
   * @param segs Array of segments to initialize the pool.
   * @param count Number of segments.
   * @return True if initialization is successful, false otherwise.
   */
  bool begin(segment* segs, uint8_t count);

  /**
   * @brief Prints the buffer content to Serial.
   * @param f Format of the output (default: 2 for binary).
   */
  void print_buffer(uint8_t f = 2);

  /**
   * @brief Prints the segment lookup table to Serial.
   * @param f Format of the output (default: 10 for decimal).
   */
  void print_segment_lookup(uint8_t f = 10);

  /**
   * @brief Prints memory pool statistics to Serial.
   */
  void print_stats();

  /**
   * @brief Allocates a memory block of the specified size.
   * @param size Size of the memory block to allocate.
   * @return Pointer to the allocated memory, or nullptr if allocation fails.
   */
  uint8_t* alloc(uint16_t size);

  /**
   * @brief Template method to allocate memory for an array of type T.
   * @param count Number of elements of type T to allocate.
   * @return Pointer to the allocated memory, or nullptr if allocation fails.
   */
  template <typename T>
  T* alloc(uint8_t count);

  /**
   * @brief Releases a previously allocated memory block.
   * @param ptr Pointer to the memory block to release.
   */
  void release(uint8_t* ptr);

  /**
   * @brief Template method to release a previously allocated memory block of type T.
   * @param ptr Pointer to the memory block to release.
   */
  template <typename T>
  void release(T* ptr);

  /**
   * @brief Return the biggest sigment size.
   */
  uint16_t max_segment_size();

 private:
  bool _initialized = false;         ///< Flag indicating if the pool is initialized
  uint8_t* _buffer = nullptr;        ///< Buffer for memory pool
  uint16_t _buffer_size = 0;         ///< Total size of the buffer
  uint8_t** _segment_ptr = nullptr;  ///< Pointers to segment starts

  uint16_t _max_segment_size = 0;      ///< Maximum segment size
  uint16_t* _segment_sizes = nullptr;  ///< Array of segment sizes
  uint8_t* _cell_count = nullptr;      ///< Array of cell counts per segment
  uint32_t* _magic_number = nullptr;   ///< Magic numbers for cell index calculation
  uint8_t* _segment_shift = nullptr;   ///< Shift values for cell index calculation

  uint8_t _segment_step = 8;          ///< Segment step size
  uint8_t _segment_count = 0;         ///< Number of segments
  int8_t* _segment_lookup = nullptr;  ///< Lookup table for segment selection
  uint8_t _segment_lookup_count = 0;  ///< Number of lookup entries

  uint16_t _total_cells = 0;  ///< Total number of cells

  /**
   * @brief Finds the first zero bit in a byte mask.
   * @param mask Byte mask to search.
   * @return Index of the first zero bit.
   */
  uint8_t _first_zero_bit(uint8_t mask);

  /**
   * @brief Finds the next segment with size greater than current.
   * @param arr Array of segments.
   * @param count Number of segments.
   * @param current Current size to compare.
   * @return Index of the next segment.
   */
  uint8_t _get_next_segment(segment* arr, uint8_t count, uint16_t current);

  /**
   * @brief Looks up the segment suitable for a given size.
   * @param size Size to find a segment for.
   * @return Index of the segment, or -1 if none found.
   */
  int8_t _lookup_segment(uint16_t size);

  /**
   * @brief Prepares a bit mask for a given cell count.
   * @param c Number of cells.
   * @return Prepared bit mask.
   */
  uint8_t _prepareMask(uint8_t c);

  uint8_t* _max_cells_used = nullptr;       ///< Maximum cells used per segment
  uint32_t _total_allocs = 0;               ///< Total number of allocations
  uint32_t _failed_allocs = 0;              ///< Number of failed allocations
  uint32_t* _allocs_per_segment = nullptr;  ///< Allocations per segment
};

#include "mempool.tpp"

extern mempool mem;