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

#define WINDOW_W 640
#define WINDOW_H 480

#define VIEW3D_X 0
#define VIEW3D_Y 0
#define VIEW3D_W WINDOW_W
#define VIEW3D_H WINDOW_H

#define MINIMAP_X 10
#define MINIMAP_Y 10
#define MINIMAP_W 160
#define MINIMAP_H 120

#define RAY_COUNT VIEW3D_W

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
  Ray rays[RAY_COUNT];
} AppState;
