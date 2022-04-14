#include "chip8.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static struct CHIP8 inter;

void chip8_init(char *path) {
  // Open file
  FILE *ptr = fopen(path, "rb");
  if (ptr == NULL) {
    printf("Can't open file!\n");
    exit(1);
  }

  for (unsigned i = 0; i < 0x1000; i++) {
    inter.c_mem[i] = 0;
  }
  for (unsigned i = 0; i < 0x10; i++) {
    inter.V[i] = 0;
    inter.stack[i] = 0;
  }

  inter.SP = 0;
  inter.I = 0;
  inter.PC = 0x200;

  fread(inter.c_mem + 0x200, 1, 0x1000 - 0x200, ptr);
  fclose(ptr);
}

void chip8_start() {
  uint8_t i = 0;
  while (i < 20) {
    uint16_t op = (inter.c_mem[inter.PC] << 8) + (inter.c_mem[inter.PC + 1]);
    uint16_t nnn = op & 0xFFF0;
    uint16_t kk = op & 0x00FF;
    uint16_t x = (op & 0x0F00) >> 8;
    uint16_t y = (op & 0x00F0) >> 4;

    printf("chip8_start: PC: %X\n", inter.PC);
    printf("chip8_start: got OP code: %04X\n", op);

    switch ((op & 0xF000) >> 12) {
    case 0:
      if (nnn == 0x0E0) {
        // CLS
        printf("chip8_start: CLS\n");
      } else if (nnn == 0x0EE) {
        // RET
        printf("chip8_start: RET\n");
      } else {
        // SYS
        printf("chip8_start: SYS\n");
      }
      break;

    case 1:
      printf("chip8_start: JP addr\n");
      break;

    case 2:
      printf("chip8_start: CALL addr\n");
      break;

    case 3:
      printf("chip8_start: SE Vx, byte\n");
      break;

    case 4:
      printf("chip8_start: SNE Vx, byte\n");
      break;

    case 5:
      printf("chip8_start: SE Vx, Vy\n");
      break;

    case 6:
      printf("chip8_start: LD Vx, byte\n");
      break;

    case 7:
      printf("chip8_start: ADD Vx, byte\n");
      break;

    case 8:
      switch (op & 0x000F) {
      case 0:
        printf("chip8_start: LD Vx, Vy\n");
        break;

      case 1:
        printf("chip8_start: OR Vx, Vy\n");
        break;

      case 2:
        printf("chip8_start: AND Vx, Vy\n");
        break;

      case 3:
        printf("chip8_start: XOR Vx, Vy\n");
        break;

      case 4:
        printf("chip8_start: ADD Vx, Vy\n");
        break;

      case 5:
        printf("chip8_start: SUB Vx, Vy\n");
        break;

      case 6:
        printf("chip8_start: SHR Vx, Vy\n");
        break;

      case 7:
        printf("chip8_start: SUBN Vx, Vy\n");
        break;

      case 0xE:
        printf("chip8_start: SHL Vx, Vy\n");
        break;

      default:
        printf("chip8_start: NO OPCODE: %X\n", op);
        break;
      }
      break;

    case 9:
      printf("chip8_start: SNE Vx, Vy\n");
      break;

    case 0xA:
      printf("chip8_start: LD I, addr\n");
      break;

    case 0xB:
      printf("chip8_start: JP V0, addr\n");
      break;

    case 0xC:
      printf("chip8_start: RND Vx, byte\n");
      break;

    case 0xD:
      printf("chip8_start: DRW Vx, byte\n");
      break;

    case 0xE:
      if (kk == 0x9E) {
        printf("chip8_start: SKP Vx\n");
      } else if (kk == 0xA1) {
        printf("chip8_start: SKNP Vx\n");
      } else {
        printf("chip8_start: NO OPCODE: %X\n", op);
      }
      break;

    case 0xF:
      switch (kk) {
      case 0x07:
        printf("chip8_start: LD Vx, DT\n");
        break;

      case 0x0A:
        printf("chip8_start: LD Vx, Keyboard\n");
        break;

      case 0x15:
        printf("chip8_start: LD DT, Vx\n");
        break;

      case 0x18:
        printf("chip8_start: LD ST, Vx\n");
        break;

      case 0x1E:
        printf("chip8_start: ADD I, Vx\n");
        break;

      case 0x29:
        printf("chip8_start: LD F, Vx\n");
        break;

      case 0x33:
        printf("chip8_start: LD B, Vx\n");
        break;

      case 0x55:
        printf("chip8_start: LD [I], Vx\n");
        break;

      case 0x65:
        printf("chip8_start: LD Vx, [I]\n");
        break;

      default:
        printf("chip8_start: NO OPCODE: %X\n", op);
        break;
      }
    default:
      printf("chip8_start: NO OPCODE: %X\n", op);
      break;
    }

    inter.PC = inter.PC + 2;
    i++;
  }
}

struct CHIP8 *chip8_inter_pointer() {
  return &inter;
}
