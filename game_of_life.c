#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "vec.h"

// length of a square cube
#define LEN 4

#define MSEC_PER_FRAME 32
#define MSEC_PER_EPOCH 10000ULL

uint32_t windowWidth = 1920;
uint32_t windowHeight = 1080;

struct Cube {
	uint32_t i;
	uint32_t j;
	bool live;
};

struct State {
	uint32_t i;
	uint32_t j;
};

// adjacency
static struct State adjs[] = {
	{.i = -1, .j = -1},
	{.i = -1, .j = 0},
	{.i = -1, .j = 1},
	{.i = 0, .j = -1},
	{.i = 0, .j = 1},
	{.i = 1, .j = -1},
	{.i = 1, .j = 0},
	{.i = 1, .j = 1},
};
	
struct Global {
	SDL_Window *window;
	SDL_Renderer *renderer;
	uint32_t width;
	uint32_t height;
	struct Cube *cubes;
	uint64_t prev;
	uint64_t epoch;
};

static struct State testStates[] = {
	{.i = 35, .j = 50},
	{.i = 36, .j = 50},
	{.i = 37, .j = 50},
	{.i = 36, .j = 49},
	{.i = 37, .j = 51},
	// extra
	{.i = 10, .j = 11},
	{.i = 11, .j = 11},
	{.i = 12, .j = 11},
	{.i = 13, .j = 11},
	{.i = 14, .j = 11},
	{.i = 20, .j = 88},
	{.i = 78, .j = 49},
	{.i = 42, .j = 21},
	{.i = 60, .j = 90},
};

int initGrid(struct Global *global)
{
	if (global->cubes != NULL)
		free(global->cubes);

	uint32_t width = global->width = windowWidth / LEN;
	uint32_t height = global->height = windowHeight / LEN;
#ifdef DEBUG
	SDL_Log("Game of Life: %u %s %u %s", height, (height == 1 || height == 0) ? "row" : "rows", width, (width == 1 || width == 0) ? "column" : "columns");
#endif

	// random initial states
	srand(time(NULL));
	uint32_t n = global->width * global->height;
	// n/3 ~ (n/2)
	uint32_t low = n / 3;
	uint32_t diff = n / 2 - low;
	uint32_t stateN = rand() % diff;
	struct State *states = malloc(sizeof(struct State) * stateN);
	if (states == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not allocate random states\n");
		return -1;
	}

	SDL_Log("number of states: %d", stateN);
	for (uint32_t i = 0; i < stateN; i++) {
		uint32_t ii = rand() % global->height;
		uint32_t jj = rand() % global->width;
		states[i].i = ii;
		states[i].j = jj;
	}

	global->cubes = calloc(height * width, sizeof(struct Global));
	for (size_t i = 0; i < stateN; i++)
		global->cubes[states[i].i * width + states[i].j].live = true;

	free(states);
	return 0;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	struct Global *global = NULL;

	*appstate = calloc(1, sizeof(struct Global));
	if (*appstate == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create global variables\n");
		return SDL_APP_FAILURE;
	}
	global = *appstate;

	if (initGrid(global)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not init the grid\n");
		return SDL_APP_FAILURE;
	}

	global->prev = SDL_GetTicks();

	global->window = SDL_CreateWindow("Game of Life", windowWidth, windowHeight, 0);
	if (global->window == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	global->renderer = SDL_CreateRenderer(global->window, NULL);
	if (global->renderer == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;
}

struct Cube *getCube(struct Global *global, uint32_t i, uint32_t j)
{
	return &global->cubes[i * global->width + j];
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	struct Global *global = appstate;
	uint32_t numberOfCubes = global->width * global->height;

	uint64_t now = SDL_GetTicks();
	if (now - global->prev < MSEC_PER_FRAME) {
		SDL_Delay(8);
		return SDL_APP_CONTINUE;
	}
	global->prev = now;


	if (now - global->epoch >= MSEC_PER_EPOCH) {
		global->epoch = now;
		initGrid(global);
	}

	SDL_SetRenderDrawColor(global->renderer, 0, 0, 0, 255);
	if (SDL_RenderClear(global->renderer) == false) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not clear the renderer, err: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// draw
	for (uint32_t k = 0; k < numberOfCubes; k++) {
		if (global->cubes[k].live == true) {
			SDL_SetRenderDrawColor(global->renderer, 255, 255, 255, 255);
			SDL_FRect rect = {
				.x = k % global->width * LEN,
				.y = k / global->width * LEN,
				.w = LEN,
				.h = LEN,
			};
			if (SDL_RenderFillRect(global->renderer, &rect) == false) {
				SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not render rectangle: %s\n", SDL_GetError());
				return SDL_APP_FAILURE;
			}
		}
	}

	// update
	struct Cube *toUpdateV = vec__new(sizeof(struct Cube));
	for (uint32_t k = 0; k < numberOfCubes; k++) {
		int i = k / global->width;
		int j = k % global->width;
		uint32_t adjLiveCnt = 0;
		struct Cube *currentCube = getCube(global, i, j);
		for (uint32_t adjI = 0; adjI < sizeof(adjs) / sizeof(struct State); adjI++) {
			int ii = i + (int)adjs[adjI].i;
			int jj = j + (int)adjs[adjI].j;
			if (ii < 0 || jj < 0)
				continue;
			else if (ii >= global->height || jj >= global->width)
				continue;
			if (getCube(global, ii, jj)->live == true)
				adjLiveCnt++;
		}
		if (currentCube->live == true) {
			if (adjLiveCnt < 2) {
				struct Cube update = {.i = i, .j = j, .live= false};

#ifdef DEBUG
				SDL_Log("under populate, die");
#endif
				vec__push(toUpdateV, update);
			} else if (adjLiveCnt > 3) {
				struct Cube update = {.i = i, .j = j, .live= false};

#ifdef DEBUG
				SDL_Log("over populate, die");
#endif
				vec__push(toUpdateV, update);
			}
		} else if (adjLiveCnt == 3) {
			struct Cube update = {.i = i, .j = j, .live= true};

#ifdef DEBUG
			SDL_Log("dead becomes live, live");
#endif
			vec__push(toUpdateV, update);
		}
	}

	for (int i = 0; i < vec__len(toUpdateV); i++) {
		struct Cube update = vec__at(toUpdateV, i);
		getCube(global, update.i, update.j)->live = update.live;
	}

	vec__free(toUpdateV);
	SDL_RenderPresent(global->renderer);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
	{
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	struct Global *global = appstate;
	SDL_DestroyWindow(global->window);
	free(global->cubes);
	free(appstate);
}
