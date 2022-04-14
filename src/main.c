#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "include/chip8.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: chip8 [path_to_rom]\n");
    exit(0);
  }
  size_t path_len = strlen(argv[1]) + 1;

  // Copy location from argv to program memory
  char buff[path_len];
  memcpy(buff, argv[1], path_len);
  buff[path_len - 1] = '\0';

  SDL_Window* win = chip8_init(buff);
  SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
  SDL_RenderSetLogicalSize(ren, 64, 32);
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
  SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
  chip8_start(win, ren, tex);

  exit(0);
}