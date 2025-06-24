#pragma once
#include "mempool.h"

/**
 * @brief Allocates memory for an array of type T.
 * @tparam T Type of the elements to allocate.
 * @param count Number of elements to allocate.
 * @return Pointer to the allocated memory, or nullptr if allocation fails.
 */
template <typename T>
T* mempool::alloc(uint8_t count) {
  uint16_t size = sizeof(T) * count;
  uint8_t* b = alloc(size);
  return reinterpret_cast<T*>(b);
}

/**
 * @brief Releases a previously allocated memory block of type T.
 * @tparam T Type of the elements to release.
 * @param ptr Pointer to the memory block to release.
 */
template <typename T>
void mempool::release(T* ptr) {
  release(reinterpret_cast<uint8_t*>(ptr));
}