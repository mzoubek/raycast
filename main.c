#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#define M_PI 3.14159265359
#include "common.h"

/* ==============================================================
 *
 *  NOTES: conversion from angle to radians = angle * M_PI / 180
 *   FIX: the conversion of flat 1D map to SDL is incorrect as boundaries exist
 *  elsewhere
 *
 * ==============================================================
 */

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static int MAP[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0,
    0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static void loadLevel(AppState *as, Level *level, SDL_FRect *rect);
static void playerMovement(AppState *as, float speed, double *dt);
static void playerRender2D(AppState *as, SDL_FRect *rect, Player *player);
static bool notWall(float x, float y, size_t tileSize, size_t tileCount,
                    int *map);
static void initializeAS(AppState *as);
static void castRays(AppState *as);

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("Martas DOOMlike", "1.0", "com.martas.doomlike");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
  initializeAS(as);
  if (!as) {
    return SDL_APP_FAILURE;
  }
  *appstate = as;

  if (!SDL_CreateWindowAndRenderer("DOOMlike", 640, 480, SDL_WINDOW_RESIZABLE,
                                   &as->window, &as->renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(as->renderer, 640, 480,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState *as = (AppState *)appstate;
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
  }

  if (event->type == SDL_EVENT_KEY_DOWN) {
    as->keys[event->key.scancode] = true;
  } else if (event->type == SDL_EVENT_KEY_UP) {
    as->keys[event->key.scancode] = false;
  }
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *as = (AppState *)appstate;

  /* --- DELTA TIME CALCULATION --- */
  Uint64 NOW = SDL_GetPerformanceCounter();
  Uint64 LAST = as->lastTime;
  double dt = ((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
  as->lastTime = NOW;

  /* Player */
  float speed = 0.3;
  SDL_FRect rect;
  SDL_FRect rectB;
  rect.w = 8;
  rect.h = 8;
  rectB.w = 60;
  rectB.h = 60;
  Player *player = &as->player;

  /* BACKGROUND COLOR */
  SDL_SetRenderDrawColor(as->renderer, 0, 0, 0,
                         SDL_ALPHA_OPAQUE); /* new color, full alpha. */

  /* clear the window to the draw color. */
  SDL_RenderClear(as->renderer);

  /* DRAWING STUFF HERE */

  playerMovement(as, speed, &dt);
  loadLevel(as, &as->level, &rectB);

  /* FOV rays */
  castRays(as);

  /* Rendering player */
  playerRender2D(as, &rect, player);

  /* put the newly-cleared rendering on the screen. */
  SDL_RenderPresent(as->renderer);

  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  /* SDL will clean up the window/renderer for us. */
  if (appstate) {
    AppState *as = (AppState *)appstate;
    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);
    SDL_free(as);
  }
}

static void loadLevel(AppState *as, Level *level, SDL_FRect *rect) {
  for (size_t i = 0; i < level->map.mapH; i++) {
    for (size_t j = 0; j < level->map.mapW; j++) {
      rect->x = level->map.pixelW * j;
      rect->y = level->map.pixelH * i;
      if (level->map.MAP[i * level->map.mapW + j] == 1) {
        SDL_SetRenderDrawColor(as->renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
      } else {
        SDL_SetRenderDrawColor(as->renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
      }

      SDL_RenderFillRect(as->renderer, rect);
    }
  }
}

// TODO: Refactor once possible if possible at all some day or other
// TODO: REFACTOR ACTUALLY NEEDED AF ASAP, I WILL HURT MYSELF IF I DON'T
// REFACTOR THIS PILE OF CRAP
static void playerMovement(AppState *as, float speed, double *dt) {
  float angle = as->player.angle * M_PI / 180;
  Player *player = &as->player;
  if (as->keys[SDL_SCANCODE_W]) {
    // for X I need new X and current Y
    float newX = cos(angle) * speed * (*dt);
    if (notWall(player->x + newX, player->y, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x + newX + 8, player->y, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x + newX, player->y + 8, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x + newX + 8, player->y + 8, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP))
      player->x += newX;
    // for Y I need new Y and current X

    float newY = sin(angle) * speed * (*dt);
    if (notWall(player->x, player->y + newY, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x, player->y + newY + 8, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x + 8, player->y + newY, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x + 8, player->y + newY + 8, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP))
      player->y += newY;
  }
  if (as->keys[SDL_SCANCODE_S]) {
    float newX = cos(angle) * speed * (*dt);
    if (notWall(player->x - newX, player->y, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x - newX + 8, player->y, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x - newX, player->y + 8, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x - newX + 8, player->y + 8, as->level.map.pixelW,
                as->level.map.mapW, as->level.map.MAP))
      player->x -= newX;

    float newY = sin(angle) * speed * (*dt);
    if (notWall(player->x, player->y - newY, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x, player->y - newY + 8, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x + 8, player->y - newY, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP) &&
        notWall(player->x + 8, player->y - newY + 8, as->level.map.pixelH,
                as->level.map.mapW, as->level.map.MAP))
      player->y -= sin(angle) * speed * (*dt);
  }
  if (as->keys[SDL_SCANCODE_A]) {
    player->angle -= speed * (*dt);
    if (player->angle < 0)
      player->angle += 360;
    if (player->angle >= 360)
      player->angle -= 360;
  }
  if (as->keys[SDL_SCANCODE_D]) {
    player->angle += speed * (*dt);
    if (player->angle < 0)
      player->angle += 360;
    if (player->angle >= 360)
      player->angle -= 360;
  }
}

static void initializeAS(AppState *as) {
  as->player.health = 100;
  as->player.armor = 0;
  as->player.ammo = 100;
  as->player.angle = 90;
  as->player.x = 200;
  as->player.y = 110;
  as->lastTime = SDL_GetPerformanceCounter();
  as->level.map.pixelW = 60;
  as->level.map.pixelH = 60;
  as->level.map.mapW = 8;
  as->level.map.mapH = 8;
  as->level.map.MAP = MAP;
}

static void castRays(AppState *as) {
  Player *player = &as->player;
  float begin = player->angle - 30;

  SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
  for (int i = 0; i < 60; i++) {
    float rayAngle = (begin + i) * M_PI / 180;
    float rayX = player->x;
    float rayY = player->y;

    while (notWall(rayX, rayY, as->level.map.pixelW, as->level.map.mapW,
                   as->level.map.MAP)) {
      rayX += cos(rayAngle) * 1;
      rayY += sin(rayAngle) * 1;
    }
    SDL_RenderLine(as->renderer, as->player.x, as->player.y, rayX, rayY);
  }
}

static void playerRender2D(AppState *as, SDL_FRect *rect, Player *player) {
  SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
  rect->x = player->x;
  rect->y = player->y;
  SDL_RenderFillRect(as->renderer, rect);

  /* POV line rendering */
  float angle = as->player.angle * M_PI / 180;
  float x2 = player->x + cos(angle) * 20;
  float y2 = player->y + sin(angle) * 20;
  SDL_RenderLine(as->renderer, player->x, player->y, x2, y2);

  /* FOV line rendering */
}

static bool notWall(float x, float y, size_t tileSize, size_t tileCount,
                    int *map) {
  int tileIdx_X = x / tileSize;
  int tileIdx_Y = y / tileSize;
  if (map[tileIdx_Y * tileCount + tileIdx_X] == 1)
    return false;

  return true;
}
