#include "chip8.h"
#include "inter.h"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <SDL2/SDL.h>

#define FREQ 60

static struct CHIP8 inter;

FILE *log_ptr;

void chip8_renderTo(uint32_t *pix) {
  for (unsigned pos = 0; pos < 64 * 32; ++pos) {
    pix[pos] = 0xFFFFFF * (inter.DispMem[pos / 8] >> (7 - pos % 8)) & 1;
  }
}

void *chip8_init(char *path) {
  // Open file
  FILE *ptr = fopen(path, "rb");
  if (ptr == NULL) {
    fprintf(log_ptr, "Can't open file!\n");
    exit(1);
  }

  log_ptr = fopen("log.txt", "w");
  if (ptr == NULL) {
    fprintf(log_ptr, "Can't open log file!\n");
    exit(1);
  }

  for (unsigned i = 0; i < 0x1000; i++) {
    inter.c_mem[i] = 0;
  }
  for (unsigned i = 0; i < 0x10; i++) {
    inter.V[i] = 0;
    inter.stack[i] = 0;
  }

  for (unsigned i = 0; i < 64 * 32 / 8; i++) {
    inter.DispMem[i] = 0;
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

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(log_ptr, "Error! Can't init SDL! (%s)\n", SDL_GetError());
  }

  SDL_Window *w =
      SDL_CreateWindow("chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       800, 400, SDL_WINDOW_RESIZABLE);

  fclose(ptr);
  return (void *)w;
}

void chip8_start(void *_win, void *_ren, void *_tex) {
  SDL_Window *win = _win;
  SDL_Renderer *ren = _ren;
  SDL_Texture *tex = _tex;

  SDL_Event ev;
  srand(time(NULL));

  uint8_t timmers = 0;

  uint32_t sTime = 0;
  uint32_t eTime = sTime;

  while (!inter.exec_break) {
    sTime = SDL_GetTicks();
    // SDL EVENTS!
    eTime = sTime + (1000 / FREQ);
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
      case SDL_QUIT:
        inter.exec_break = 1;
        break;
      case SDL_WINDOWEVENT:
        switch (ev.window.event) {
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_MINIMIZED:
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          inter.drawFlag = 1;
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }

      /*if (ev.type == SDL_QUIT) {
        inter.exec_break = 1;
        break;
      }*/
    }
    // Step
    chip8_step(&inter, log_ptr);

    // Check for drawFlag
    if (inter.drawFlag == 1) {
      SDL_Rect r;
      uint8_t x, y;

      r.x = r.y = 0;
      r.w = r.h = 1;

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
    // getchar();

    /**inter.PC = inter.PC + 2;
     *i++;
     * Used for testing, not needed anymore.
     **/

    timmers++;
    if (timmers == 60) {
      inter.delay = (inter.delay > 0) ? inter.delay-- : 0;
      timmers = 0;
    }

    uint32_t now = SDL_GetTicks();
    uint32_t _t = (now >= eTime) ? 0 : (eTime - now);

    SDL_Delay(_t);
  }
}

struct CHIP8 *chip8_inter_pointer() {
  return &inter;
}
