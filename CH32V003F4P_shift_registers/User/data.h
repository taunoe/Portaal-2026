#include "debug.h"

#define DATA_SIZE 32  // 4pc * 8-bit Sihft Register

typedef struct {
    uint32_t data[DATA_SIZE];
    volatile size_t head; // Kirjutatakse
    volatile size_t tail; // Loetakse
} Data32;


