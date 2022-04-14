#include "chip8.h"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

static struct CHIP8 inter;

void chip8_renderTo(uint32_t *pix) {
  for (unsigned pos = 0; pos < 64 * 32; ++pos) {
    pix[pos] = 0xFFFFFF * (inter.DispMem[pos / 8] >> (7 - pos % 8)) & 1;
  }
}

void *chip8_init(char *path) {
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

  inter.drawFlag = 0;

  uint8_t _fontset[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  for (uint8_t i = 0; i < 80; i++) {
    inter.c_mem[0x50 + i] = _fontset[i];
  }

  fread(inter.c_mem + 0x200, 1, 0x1000 - 0x200, ptr);
  fclose(ptr);

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("Error! Can't init SDL! (%s)\n", SDL_GetError());
  }

  SDL_Window *w = SDL_CreateWindow("chip8", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, 800, 400, 0);

  return (void *)w;
}

void chip8_start(void *_win, void *_ren, void *_tex) {
  SDL_Window *win = _win;
  SDL_Renderer *ren = _ren;
  SDL_Texture *tex = _tex;

  SDL_Event ev;
  srand(time(NULL));

  uint8_t _break = 0;
  while (!_break) {
    // SDL EVENTS!
    while (SDL_PollEvent(&ev))
      if (ev.type == SDL_QUIT) {
        _break = 1;
        break;
      }

    uint16_t op = (inter.c_mem[inter.PC] << 8) + (inter.c_mem[inter.PC + 1]);
    uint16_t nnn = op & 0x0FFF;
    uint16_t kk = op & 0x00FF;
    uint16_t x = (op & 0x0F00) >> 8;
    uint16_t y = (op & 0x00F0) >> 4;
    uint16_t nib = op & 0x000F;

    printf("chip8_start: PC: %X\n", inter.PC);
    printf("chip8_start: got OP code: %04X\n", op);

    switch ((op & 0xF000) >> 12) {
    case 0:
      if (nnn == 0x0E0) {
        // CLS
        printf("chip8_start: CLS\n");

        inter.PC += 2;
      } else if (nnn == 0x0EE) {
        // RET
        printf("chip8_start: RET\n");
      } else {
        // SYS
        printf("chip8_start: SYS\n");
        printf("Stoping exec...\n");

        _break = 1;
      }
      break;

    case 1:
      printf("chip8_start: JP addr\n");
      inter.PC = nnn;
      break;

    case 2:
      printf("chip8_start: CALL addr\n");
      break;

    case 3:
      printf("chip8_start: SE Vx, byte\n");
      inter.PC = inter.PC + ((inter.V[x] == kk) ? 4 : 2);
      break;

    case 4:
      printf("chip8_start: SNE Vx, byte\n");
      inter.PC = inter.PC + ((inter.V[x] != kk) ? 4 : 2);
      break;

    case 5:
      printf("chip8_start: SE Vx, Vy\n");
      inter.PC = inter.PC + ((inter.V[x] == inter.V[y]) ? 4 : 2);
      break;

    case 6:
      printf("chip8_start: LD Vx, byte\n");
      inter.V[x] = kk;
      inter.PC += 2;
      break;

    case 7:
      printf("chip8_start: ADD Vx, byte\n");
      inter.V[x] = inter.V[x] + kk;
      inter.PC += 2;
      break;

    case 8:
      switch (op & 0x000F) {
      case 0:
        printf("chip8_start: LD Vx, Vy\n");
        inter.V[x] = inter.V[y];
        inter.PC += 2;
        break;

      case 1:
        printf("chip8_start: OR Vx, Vy\n");
        inter.V[x] = inter.V[x] | inter.V[y];
        inter.PC += 2;
        break;

      case 2:
        printf("chip8_start: AND Vx, Vy\n");
        inter.V[x] = inter.V[x] & inter.V[y];
        inter.PC += 2;
        break;

      case 3:
        printf("chip8_start: XOR Vx, Vy\n");
        inter.V[x] = inter.V[x] ^ inter.V[y];
        inter.PC += 2;
        break;

      case 4:
        printf("chip8_start: ADD Vx, Vy\n");
        uint8_t tmp = inter.V[x];
        inter.V[x] = inter.V[x] + inter.V[y];
        if (tmp > inter.V[x])
          inter.V[0xF] = 1;
        else
          inter.V[0xF] = 0;
        inter.PC += 2;
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
      inter.I = nnn;
      inter.PC += 2;
      break;

    case 0xB:
      printf("chip8_start: JP V0, addr\n");
      break;

    case 0xC:
      printf("chip8_start: RND Vx, byte\n");
      inter.V[x] = (rand() % 0x100) & kk;
      inter.PC += 2;
      break;

    case 0xD:
      printf("chip8_start: DRW Vx, Vy, nibble\n");
      inter.V[0xF] = 0;

      uint8_t pixel;
      inter.V[0xF] = 0;
      for (int yline = 0; yline < nib; yline++) {
        pixel = inter.c_mem[inter.I + yline];
        for (int xline = 0; xline < 8; xline++) {
          if ((pixel & (0x80 >> xline)) != 0) {
            if (inter.DispMem[(inter.V[x] + xline + ((inter.V[y] + yline) * 64))] == 1) {
              inter.V[0xF] = 1;
            }
            inter.DispMem[inter.V[x] + xline + ((inter.V[y] + yline) * 64)] ^= 1;
          }
        }
      }

      inter.drawFlag = 1;
      inter.PC += 2;
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

    // Check for drawFlag
    if (inter.drawFlag == 1) {
      SDL_Rect r;
      uint8_t x, y;

      r.x = 0;
      r.y = 0;

      r.w = 1;
      r.h = 1;

      SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
      SDL_RenderClear(ren);
      SDL_SetRenderDrawColor(ren, 0, 255, 0, 0);
      for (x = 0; x < 64; x++) {
        for (y = 0; y < 32; y++) {
          if (inter.DispMem[(x) + ((y)*64)] == 1) {
            r.x = x;
            r.y = y;
            SDL_RenderFillRect(ren, &r);
          }
        }
      }

      SDL_RenderPresent(ren);
      inter.drawFlag = 0;
    }

    // Used for debuging!
    //getchar();

    /**inter.PC = inter.PC + 2;
     *i++;
     * Used for testing, not needed anymore.
     **/
  }
}

struct CHIP8 *chip8_inter_pointer() {
  return &inter;
}
