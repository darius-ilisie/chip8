#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  chip8_init(buff);
  chip8_start();

  exit(0);
}