#include "inter.h"
#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>

void chip8_step(struct CHIP8 *inter, void *log_ptr) {

  uint16_t op = (inter->c_mem[inter->PC] << 8) + (inter->c_mem[inter->PC + 1]);
  uint16_t nnn = op & 0x0FFF;
  uint16_t kk = op & 0x00FF;
  uint16_t x = (op & 0x0F00) >> 8;
  uint16_t y = (op & 0x00F0) >> 4;
  uint16_t nib = op & 0x000F;

  fprintf(log_ptr, "\nchip8_start: PC: %X\n", inter->PC);
  fprintf(log_ptr, "chip8_start: got OP code: %04X\n", op);

  switch ((op & 0xF000) >> 12) {

  case 0:
    if (nnn == 0x0E0) {
      // CLS
      fprintf(log_ptr, "chip8_start: CLS\n");
      for (unsigned i = 0; i < 64 * 32 / 8; i++) {
        inter->c_mem[i] = 0;
      }
      inter->drawFlag = 1;
      inter->PC += 2;
    } else if (nnn == 0x0EE) {
      // RET
      fprintf(log_ptr, "chip8_start: RET\n");
      inter->SP--;
      inter->PC = inter->stack[inter->SP] +
                 2; // We add +2 so we get the next opcode after CALL
    } else {
      // SYS
      fprintf(log_ptr, "chip8_start: SYS\n");
      fprintf(log_ptr, "Stoping exec...\n");

      inter->exec_break = 1;
    }
    break;

  case 1:
    fprintf(log_ptr, "chip8_start: JP addr\n");
    inter->PC = nnn;
    break;

  case 2:
    fprintf(log_ptr, "chip8_start: CALL addr\n");
    inter->stack[inter->SP] = inter->PC;
    inter->SP++;

    inter->PC = nnn;
    break;

  case 3:
    fprintf(log_ptr, "chip8_start: SE Vx, byte\n");
    inter->PC = inter->PC + ((inter->V[x] == kk) ? 4 : 2);
    break;

  case 4:
    fprintf(log_ptr, "chip8_start: SNE Vx, byte\n");
    inter->PC = inter->PC + ((inter->V[x] != kk) ? 4 : 2);
    break;

  case 5:
    fprintf(log_ptr, "chip8_start: SE Vx, Vy\n");
    inter->PC = inter->PC + ((inter->V[x] == inter->V[y]) ? 4 : 2);
    break;

  case 6:
    fprintf(log_ptr, "chip8_start: LD Vx, byte\n");
    inter->V[x] = kk;
    inter->PC += 2;
    break;

  case 7:
    fprintf(log_ptr, "chip8_start: ADD Vx, byte\n");
    inter->V[x] = inter->V[x] + kk;
    inter->PC += 2;
    break;

  case 8:
    switch (op & 0x000F) {
    case 0:
      fprintf(log_ptr, "chip8_start: LD Vx, Vy\n");
      inter->V[x] = inter->V[y];
      inter->PC += 2;
      break;

    case 1:
      fprintf(log_ptr, "chip8_start: OR Vx, Vy\n");
      inter->V[x] = inter->V[x] | inter->V[y];
      inter->PC += 2;
      break;

    case 2:
      fprintf(log_ptr, "chip8_start: AND Vx, Vy\n");
      inter->V[x] = inter->V[x] & inter->V[y];
      inter->PC += 2;
      break;

    case 3:
      fprintf(log_ptr, "chip8_start: XOR Vx, Vy\n");
      inter->V[x] = inter->V[x] ^ inter->V[y];
      inter->PC += 2;
      break;

    case 4:
      fprintf(log_ptr, "chip8_start: ADD Vx, Vy\n");
      uint8_t tmp = inter->V[x];
      inter->V[x] = inter->V[x] + inter->V[y];
      inter->V[0xF] = (tmp > inter->V[x]) ? 1 : 0;
      inter->PC += 2;
      break;

    case 5:
      fprintf(log_ptr, "chip8_start: SUB Vx, Vy\n");
      inter->V[0xF] = (inter->V[x] > inter->V[y]) ? 1 : 0;
      inter->V[x] = inter->V[x] - inter->V[y];
      inter->PC += 2;
      break;

    case 6:
      fprintf(log_ptr, "chip8_start: SHR Vx, {Vy}\n");
      // Shift Right
      inter->V[0xF] = (inter->V[x] & 0b1);
      inter->V[x] = inter->V[x] >> 1;
      inter->PC += 2;
      break;

    case 7:
      fprintf(log_ptr, "chip8_start: SUBN Vx, Vy\n");
      inter->V[0xF] = (inter->V[y] > inter->V[x]) ? 1 : 0;
      inter->V[x] = inter->V[y] - inter->V[x];
      inter->PC += 2;
      break;

    case 0xE:
      fprintf(log_ptr, "chip8_start: SHL Vx, {Vy}\n");
      // Left shift
      inter->V[0xF] = ((inter->V[x] & 0b10000000) >> 7);
      inter->V[x] = inter->V[x] << 1;
      inter->PC += 2;
      break;

    default:
      // fprintf(log_ptr, "chip8_start: NO OPCODE: %X\n", op);
      break;
    }
    break;

  case 9:
    fprintf(log_ptr, "chip8_start: SNE Vx, Vy\n");
    if (inter->V[x] != inter->V[y])
      inter->PC += 2;
    inter->PC += 2;
    break;

  case 0xA:
    fprintf(log_ptr, "chip8_start: LD I, addr\n");
    inter->I = nnn;
    inter->PC += 2;
    break;

  case 0xB:
    fprintf(log_ptr, "chip8_start: JP V0, addr\n");
    break;

  case 0xC:
    fprintf(log_ptr, "chip8_start: RND Vx, byte\n");
    inter->V[x] = (rand() % 0x100) & kk;
    inter->PC += 2;
    break;

  case 0xD:
    fprintf(log_ptr, "chip8_start: DRW Vx, Vy, nibble\n");
    inter->V[0xF] = 0;

    uint8_t pixel;
    inter->V[0xF] = 0;
    for (int yline = 0; yline < nib; yline++) {
      pixel = inter->c_mem[inter->I + yline];
      for (int xline = 0; xline < 8; xline++) {
        if ((pixel & (0x80 >> xline)) != 0) {
          if (inter->DispMem[(inter->V[x] + xline +
                             ((inter->V[y] + yline) * 64))] == 1) {
            inter->V[0xF] = 1;
          }
          inter->DispMem[inter->V[x] + xline + ((inter->V[y] + yline) * 64)] ^= 1;
        }
      }
    }

    inter->drawFlag = 1;
    inter->PC += 2;
    break;

  case 0xE:
    if (kk == 0x9E) {
      fprintf(log_ptr, "chip8_start: SKP Vx\n");
    } else if (kk == 0xA1) {
      fprintf(log_ptr, "chip8_start: SKNP Vx\n");
    } else {
      // fprintf(log_ptr, "chip8_start: NO OPCODE: %X\n", op);
    }
    break;

  case 0xF:
    switch (kk) {
    case 0x07:
      fprintf(log_ptr, "chip8_start: LD Vx, DT\n");
      inter->V[x] = inter->delay;

      inter->PC += 2;
      break;

    case 0x0A:
      fprintf(log_ptr, "chip8_start: LD Vx, Keyboard\n");
      break;

    case 0x15:
      fprintf(log_ptr, "chip8_start: LD DT, Vx\n");
      inter->delay = inter->V[x];

      inter->PC += 2;
      break;

    case 0x18:
      fprintf(log_ptr, "chip8_start: LD ST, Vx\n");
      break;

    case 0x1E:
      fprintf(log_ptr, "chip8_start: ADD I, Vx\n");
      inter->I = inter->I + inter->V[x];

      inter->PC += 2;
      break;

    case 0x29:
      fprintf(log_ptr, "chip8_start: LD F, Vx\n");
      break;

    case 0x33:
      fprintf(log_ptr, "chip8_start: LD B, Vx\n");
      inter->c_mem[inter->I] = inter->V[x] / 100;
      inter->c_mem[inter->I + 1] = (inter->V[x] / 10) % 10;
      inter->c_mem[inter->I + 2] = inter->V[x] % 10;

      inter->PC += 2;
      break;

    case 0x55:
      fprintf(log_ptr, "chip8_start: LD [I], Vx\n");
      for (uint8_t offset = 0; offset <= x; offset++) {
        inter->c_mem[inter->I + offset] = inter->V[offset];
      }

      inter->PC += 2;
      break;

    case 0x65:
      fprintf(log_ptr, "chip8_start: LD Vx, [I]\n");
      for (uint8_t offset = 0; offset <= x; offset++) {
        inter->V[offset] = inter->c_mem[inter->I + offset];
      }

      inter->PC += 2;
      break;

    default:
      // fprintf(log_ptr, "chip8_start: NO OPCODE: %X\n", op);
      break;
    }
  default:
    // fprintf(log_ptr, "chip8_start: NO OPCODE: %X\n", op);
    break;
  }
}