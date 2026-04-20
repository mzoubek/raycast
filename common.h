#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: check later if this needs to be a pointer

typedef struct {
  float rayX;
  float rayY;
  float distance;
} Ray;

typedef struct {
  uint8_t health;
  uint8_t armor;
  uint8_t ammo;
  float angle;
  float x;
  float y;
} Player;

typedef struct {
  size_t pixelW;
  size_t pixelH;
  size_t mapW;
  size_t mapH;
  int *MAP;
} Map;

typedef struct {
  Player *player;
  Map map;
} Level;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  bool keys[SDL_SCANCODE_COUNT];
  Player player;
  Uint64 lastTime;
  Level level;
  Ray rays[320];
} AppState;
