#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

struct CHIP8 {
  uint8_t c_mem[0x1000];
  uint8_t V[0x10]; // V0 -> VF
  uint8_t SP;
  uint16_t stack[0x10];
  uint16_t I;
  uint16_t PC;

  uint8_t delay;
  uint8_t timer;

  uint8_t DispMem[64 * 32 / 8];
  uint8_t drawFlag;
};

void *chip8_init(char *);
void chip8_start(void *, void *, void *);

void chip8_renderTo(uint32_t *);
struct CHIP8 *chip8_inter_pointer();

#endif