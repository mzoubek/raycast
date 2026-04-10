#include <SDL3/SDL_render.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include "common.h"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("Martas DOOMlike", "1.0", "com.martas.doomlike");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
  as->player.health = 100;
  as->player.armor = 0;
  as->player.ammo = 100;
  as->player.angle = 0;
  as->player.x = 320;
  as->player.y = 240;
  as->lastTime = SDL_GetPerformanceCounter();
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
  float speed = 5;
  SDL_FRect rect;
  rect.w = 25;
  rect.h = 25;
  Player *player = &as->player;

  /* BACKGROUND COLOR */
  SDL_SetRenderDrawColor(as->renderer, 0, 0, 0,
                         SDL_ALPHA_OPAQUE); /* new color, full alpha. */

  /* clear the window to the draw color. */
  SDL_RenderClear(as->renderer);

  /* DRAWING STUFF HERE */
  if (as->keys[SDL_SCANCODE_W]) {
    player->y -= speed * dt;
  } else if (as->keys[SDL_SCANCODE_S]) {
    player->y += speed * dt;
  } else if (as->keys[SDL_SCANCODE_A]) {
    player->x -= speed * dt;
  } else if (as->keys[SDL_SCANCODE_D]) {
    player->x += speed * dt;
  }

  SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
  rect.x = player->x;
  rect.y = player->y;
  SDL_RenderFillRect(as->renderer, &rect);

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
