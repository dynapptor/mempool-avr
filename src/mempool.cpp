#include "mempool.h"

#include <Arduino.h>

mempool::mempool() {}
mempool::~mempool() {
  clean();
}

void mempool::clean() {
  if (_segment_sizes) delete[] _segment_sizes;
  if (_cell_count) delete[] _cell_count;
  if (_magic_number) delete[] _magic_number;
  if (_segment_shift) delete[] _segment_shift;
  if (_buffer) delete[] _buffer;
  if (_segment_lookup) delete[] _segment_lookup;
  if (_segment_ptr) delete[] _segment_ptr;
#ifdef MEMPOOL_DEBUG
  if (_max_cells_used) delete[] _max_cells_used;
  if (_allocs_per_segment) delete[] _allocs_per_segment;
#endif
}

const uint8_t first_zero_bit_lut[256] PROGMEM = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

uint8_t mempool::_first_zero_bit(uint8_t mask) {
  return pgm_read_byte(&first_zero_bit_lut[mask]);
}

bool mempool::begin(segment* segs, uint8_t count) {
  if (_initialized) return false;
  if (count > 32) return false;
  _segment_sizes = new uint16_t[count];
  if (!_segment_sizes) {
    clean();
    return false;
  }
  _cell_count = new uint8_t[count];
  if (!_cell_count) {
    clean();
    return false;
  }
  _magic_number = new uint32_t[count];
  if (!_magic_number) {
    clean();
    return false;
  }
  _segment_shift = new uint8_t[count]{};
  if (!_segment_shift) {
    clean();
    return false;
  }
#ifdef MEMPOOL_DEBUG
  _max_cells_used = new uint8_t[count]{};
  if (!_max_cells_used) {
    clean();
    return false;
  }
  _allocs_per_segment = new uint32_t[count]{};
  if (!_allocs_per_segment) {
    clean();
    return false;
  }
#endif
  _initialized = true;
  _segment_count = count;
  _buffer_size = 0;

  uint8_t ix;
  uint16_t currentSize = 0;
  for (uint8_t i = 0; i < count; ++i) {
    if (segs[i].size == 0) {
      clean();
      return false;
    }
    ix = _get_next_segment(segs, count, currentSize);

    _segment_sizes[i] = segs[ix].size * SEGMENT_STEP;
    if (_segment_sizes[i] > 64) {
      clean();
      return false;
    }
    _cell_count[i] = segs[ix].count;

    currentSize = segs[ix].size;
    _buffer_size += _segment_sizes[i] * _cell_count[i];  // data
    _buffer_size += (_cell_count[i] + 7) / 8;            // for pool masks
    _buffer_size++;                                      // pool header mask bytes
  }

  _max_segment_size = _segment_sizes[count - 1];

  _buffer = new uint8_t[_buffer_size]{};
  if (!_buffer) {
    clean();
    return false;
  }
  _segment_lookup_count = (_max_segment_size / SEGMENT_STEP);
  _segment_lookup = new int8_t[_segment_lookup_count];
  if (!_segment_lookup) {
    clean();
    return false;
  }

  for (uint8_t i = 1; i <= _segment_lookup_count; i++) {
    _segment_lookup[i - 1] = _lookup_segment(i * SEGMENT_STEP);
  }

  uint8_t* p = _buffer;
  _segment_ptr = new uint8_t*[count];
  if (!_segment_ptr) {
    clean();
    return false;
  }

  for (uint8_t i = 0; i < count; ++i) {
    p += (_cell_count[i] + 7) / 8;
    p++;
    _segment_ptr[i] = p;
    p += _segment_sizes[i] * _cell_count[i];
    if (_segment_sizes[i] & (_segment_sizes[i] - 1)) {
      _magic_number[i] = (65536 + (_segment_sizes[i] >> 2) - 1) / (_segment_sizes[i] >> 2);
      _segment_shift[i] = 16;
    } else {
      _magic_number[i] = 1;
      _segment_shift[i] = __builtin_ctz(_segment_sizes[i]);
    }
  }

  for (uint8_t i = 0; i < count; i++) {
    p = _segment_ptr[i];
    p--;
    *p = _prepareMask((_cell_count[i] + 7) / 8);
    p -= (_cell_count[i] + 7) / 8;
    *p = _prepareMask(_cell_count[i] % 8);
  }
  return true;
}

uint8_t mempool::_get_next_segment(segment* arr, uint8_t count, uint16_t current) {
  uint16_t find = -1;
  uint8_t ret = 0;
  for (uint8_t i = 0; i < count; i++) {
    if (arr[i].size > current && arr[i].size < find) {
      ret = i;
      find = arr[i].size;
    }
  }
  return ret;
}

int8_t mempool::_lookup_segment(uint16_t size) {
  for (uint8_t i = 0; i < _segment_count; i++) {
    if (_segment_sizes[i] >= size) return i;
  }
  return -1;
}

uint8_t mempool::_prepareMask(uint8_t c) {
  if (c == 0) return 0;
  uint8_t ret = 0xFF;
  ret = ret << c;
  return ret;
}

void mempool::print_buffer(uint8_t f) {
  for (uint16_t i = 0; i < _buffer_size; i++) {
    Serial.print(_buffer[i], f);
    Serial.print(' ');
  }
  Serial.println();
}

void mempool::print_segment_lookup(uint8_t f) {
  for (uint8_t i = 0; i < _segment_lookup_count; i++) {
    Serial.print(_segment_lookup[i], f);
    Serial.print(' ');
  }
  Serial.println();
}

uint8_t* mempool::alloc(uint16_t size) {
  if (size > _max_segment_size) {
#ifdef MEMPOOL_DEBUG
    _failed_allocs++;
#endif
    return nullptr;
  }
  uint8_t sg = _segment_lookup[((size + SEGMENT_STEP - 1) >> SEGMENT_LOG2) - 1];
  uint8_t* base = _segment_ptr[sg];
  if (base[-1] == 0xFF) {
    if (sg < _segment_count - 1) return alloc(_segment_sizes[sg + 1]);
#ifdef MEMPOOL_DEBUG
    _failed_allocs++;
#endif
    return nullptr;
  }
  uint8_t poolIndex = _first_zero_bit(~base[-1]);
  uint8_t* cellBase = base - (poolIndex + 2);
  uint8_t cellIndex = _first_zero_bit(~cellBase[0]);
  bitSet(*cellBase, cellIndex);
  if (*cellBase == 0xFF) bitSet(*(base - 1), poolIndex);

#ifdef MEMPOOL_DEBUG
  _total_allocs++;
  _allocs_per_segment[sg]++;
  uint8_t used_cells = 0;
  for (uint8_t i = 0; i < (_cell_count[sg] + 7) / 8; i++) {
    used_cells += 8 - _first_zero_bit(~(cellBase[i - poolIndex - 2]));
  }
  if (used_cells > _max_cells_used[sg]) _max_cells_used[sg] = used_cells;
#endif
  uint16_t offset;
  uint16_t index = (poolIndex << 3) + cellIndex;
  asm volatile(
      "mul %A0, %A1 \n\t"
      "movw %A2, r0 \n\t"
      "mul %B0, %A1 \n\t"
      "add %B2, r0 \n\t"
      : "=r"(offset)
      : "r"(index), "r"(_segment_sizes[sg])
      : "r0", "r1");
  return base + offset;
}

void mempool::release(uint8_t* ptr) {
  int8_t sg = -1;
  uint8_t l = 0, r = _segment_count - 1;
  while (l <= r) {
    uint8_t m = (l + r) >> 1;
    if (ptr < _segment_ptr[m]) {
      r = m - 1;
    } else {
      sg = m;
      l = m + 1;
    }
  }
  if (sg == -1) return;
  uint8_t* base = _segment_ptr[sg];
  uint16_t offset = ptr - base;
  uint8_t cellIndex;
  if (_segment_sizes[sg] & (_segment_sizes[sg] - 1)) {
    uint16_t temp = offset >> 2;
    asm volatile(
        "mul %A0, %A1 \n\t"
        "movw %A2, r0 \n\t"
        "mul %B0, %B1 \n\t"
        "movw %C2, r0 \n\t"
        "mul %A0, %B1 \n\t"
        "add %B2, r0 \n\t"
        "adc %C2, r1 \n\t"
        "mul %B0, %A1 \n\t"
        "add %B2, r0 \n\t"
        "adc %C2, r1 \n\t"
        "mov %A0, %C2 \n\t"
        : "=r"(cellIndex)
        : "r"(temp), "r"(_magic_number[sg])
        : "r0", "r1");
  } else {
    cellIndex = offset >> _segment_shift[sg];
  }
  uint8_t poolIndex = cellIndex >> 3;
  uint8_t bitIndex = cellIndex & 0x07;
  base -= poolIndex + 2;
  bitClear(*base, bitIndex);
  bitClear(*(base + poolIndex + 1), poolIndex);
}

void mempool::print_stats() {
  if (!Serial) return;
#ifdef MEMPOOL_DEBUG
  Serial.print("Total allocs: ");
  Serial.println(_total_allocs);
  Serial.print("Failed allocs: ");
  Serial.println(_failed_allocs);
  for (uint8_t i = 0; i < _segment_count; i++) {
    Serial.print("Segment ");
    Serial.print(i);
    Serial.print(": max cells used = ");
    Serial.print(_max_cells_used[i]);
    Serial.print(", allocs = ");
    Serial.println(_allocs_per_segment[i]);
  }
#else
  Serial.println("Debug stats not available. Enable MEMPOOL_DEBUG to see statistics.");
#endif
}

/*

uint8_t* mempool::alloc(uint16_t size) {
  if (size > _max_segment_size) return nullptr;
  uint8_t sg = _segment_lookup[((size + SEGMENT_STEP - 1) >> SEGMENT_LOG2) - 1];
  uint8_t* p = _segment_ptr[sg];
  if (p[-1] == 0xFF) {
    if (sg < _segment_count - 1) {
      return alloc(_segment_sizes[sg + 1]);
    } else {
      return nullptr;
    }
  }
  uint8_t poolIndex = _first_zero_bit(~p[-1]);
  uint8_t cellIndex = _first_zero_bit(~p[-(poolIndex + 2)]);
  p -= poolIndex + 2;
  bitSet(*p, cellIndex);
  if (*p == 0xFF) {
    p = _segment_ptr[sg] - 1;
    bitSet(*p, poolIndex);
  }
  p = _segment_ptr[sg];
  return &p[(poolIndex * 8 + cellIndex) * _segment_sizes[sg]];
}

void mempool::release(uint8_t* ptr) {
  // Bináris keresés a szegmenshez
  int8_t sg = -1;
  uint8_t l = 0, r = _segment_count - 1;
  while (l <= r) {
    uint8_t m = (l + r) >> 1;
    if (ptr < _segment_ptr[m]) {
      r = m - 1;
    } else {
      sg = m;
      l = m + 1;
    }
  }
  if (sg == -1) return;  // Invalid pointer

  uint8_t* base = _segment_ptr[sg];
  uint16_t offset = ptr - base;
  uint8_t cellIndex;
  if (_segment_sizes[sg] & (_segment_sizes[sg] - 1)) {
    cellIndex = ((offset >> 2) * _magic_number[sg]) >> 16;
  } else {
    cellIndex = offset >> _segment_shift[sg];
  }
  uint8_t poolIndex = cellIndex >> 3;
  uint8_t bitIndex = cellIndex & 0x07;
  base--;
  bitClear(*base, poolIndex);
  base -= (poolIndex + 1);
  bitClear(*base, bitIndex);
}

*/