#include <Arduino.h>
#include <mempool.h>

mempool pool;
segment segments[] = {
    segment(10, 8),  // 10 cells of 8 bytes
    segment(5, 16),  // 5 cells of 16 bytes
    segment(3, 32)   // 3 cells of 32 bytes
};

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  // Initialize the memory pool
  if (!pool.begin(segments, 3)) {
    Serial.println("Memory pool initialization failed!");
    while (1);
  }

  // Allocate memory
  uint8_t* buf1 = pool.alloc(12);  // Should use 16-byte segment
  if (buf1) {
    Serial.println("Allocated 12 bytes");
  }

  int* buf2 = pool.alloc<int>(2);  // Allocate 2 integers (8 bytes)
  if (buf2) {
    buf2[0] = 42;
    buf2[1] = 84;
    Serial.print("Allocated 2 integers: ");
    Serial.print(buf2[0]);
    Serial.print(", ");
    Serial.println(buf2[1]);
  }

  // Release memory
  pool.release(buf1);
  pool.release(buf2);

  // Print statistics
  pool.print_stats();
}

void loop() {
  // Nothing to do here
}