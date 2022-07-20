#include "SDL.h"
#include "SDL_image.h"
#include "SlidePuzzle.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static SPUZ_Board p;
static bool Running = true;
static bool NewGame = true;
static Uint16 PressedX = 0;
static Uint16 PressedY = 0;
static bool SpaceHeld = false;
static SDL_Window* Window = NULL;
static SDL_Renderer* Renderer = NULL;
static SDL_Texture* Picture = NULL;
static SDL_Texture* Arrows = NULL;

void Init();
void MainLoop();
void ProcessEvents();
void Update();
void RenderPuzzle();
void RenderSolved(bool solved);
void Quit(const char* const format, ...);

#define TILE_SIZE 256
#define BOARD_SIZE (256 * 4)
#define SOLVED_DELAY 5000

int main(int argc, char** argv) {
	Init();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(MainLoop, 60, 1);
#else
	while (Running) {
		MainLoop();
	}
#endif

	Quit(NULL);
	return 0;
}

void MainLoop() {
	if (NewGame) {
		/* In the odd event permuting the panels results in a solved puzzle,
		 * try again until it isn't solved. The permuting function doesn't
		 * guarantee the puzzle is not in the solved state. */
		do {
			SPUZ_Permute(&p, (unsigned)time(NULL));
		} while (SPUZ_Solved(&p));

		NewGame = 0;
	}
	ProcessEvents();
	if (SpaceHeld) {
		RenderSolved(false);
	}
	else {
		if (Running) Update();
		RenderPuzzle();
	}
	if (SPUZ_Solved(&p)) {
		Uint32 endTicks = SDL_GetTicks() + SOLVED_DELAY;
		while (Running && SDL_GetTicks() < endTicks) {
			ProcessEvents();
			RenderSolved(true);
		}
		NewGame = 1;
	}
}

void Init(void) {
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Controls", "Using the mouse, click squares next to the empty square to move.\nHold space bar to preview the solved puzzle.", NULL) < 0) {
		Quit("Unable to show controls message:\n%s", SDL_GetError());
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		Quit("Unable to initialize SDL:\n%s", SDL_GetError());
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	SDL_DisplayMode mode;
	if (SDL_GetDesktopDisplayMode(0, &mode) < 0) {
		Quit("Unable to get desktop display mode:\n%s", SDL_GetError());
	}
	const int windowSize = ((mode.w > mode.h ? mode.h : mode.w) * 2) / 3;

	if ((Window = SDL_CreateWindow("Slide Puzzle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowSize, windowSize, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE)) == NULL) {
		Quit("Unable to create SDL window:\n%s", SDL_GetError());
	}
	
	if ((Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
		Quit("Unable to create SDL renderer:\n%s", SDL_GetError());
	}

	if (SDL_RenderSetLogicalSize(Renderer, BOARD_SIZE, BOARD_SIZE) < 0) {
		Quit("Unable to set SDL renderer logical size:\n%s", SDL_GetError());
	}

#define LOADIMG(image, type) \
	if ((image = IMG_LoadTexture(Renderer, #image "." type)) == NULL) { \
		Quit("Unable to load \"" #image "." type "\" image:\n%s", SDL_GetError()); \
	}
	LOADIMG(Picture, "jpg");
	LOADIMG(Arrows, "png");
#undef LOADIMG
	
	SDL_ShowWindow(Window);

	SPUZ_Init(&p);
}

void ProcessEvents(void) {
	SDL_Event e;

	SDL_PumpEvents();
	while (SDL_PeepEvents(&e, 1, SDL_GETEVENT, 0, SDL_LASTEVENT) == 1) {
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN:
				PressedX = e.button.x;
				PressedY = e.button.y;
				break;
			case SDL_MOUSEBUTTONUP:
				PressedX = -1;
				PressedY = -1;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_SPACE) {
					SpaceHeld = true;
				}
				break;
			case SDL_KEYUP:
				if (e.key.keysym.sym == SDLK_SPACE) {
					SpaceHeld = false;
				}
				break;
			case SDL_QUIT:
				Running = 0;
				return;
			default:
				break;
		}
	}
}

void Update() {
	const Sint16 x = p.emptyX * TILE_SIZE;
	const Sint16 y = p.emtpyY * TILE_SIZE;
	SPUZ_Direction dir = SPUZ_INVALIDDIRECTION;

	/*
	 * SDL/board coordinates:
	 * 012345...
	 * 1
	 * 2
	 * 3
	 * 4
	 * 5
	 * .
	 * .
	 * .
	 */
	if (PressedX >= 0 && PressedX < BOARD_SIZE && PressedY >= 0 && PressedY < BOARD_SIZE) {
		// Either case of if/else below could be inside empty panel
		// In that case, dir remains set to SPUZ_INVALIDDIRECTION

		// Could be above or below empty panel
		if (PressedX > x && PressedX < x + TILE_SIZE) {
			// Below empty panel
			if (PressedY > y + TILE_SIZE && PressedY <= y + TILE_SIZE * 2) {
				dir = SPUZ_UP;
			}
			// Above empty panel
			else if (PressedY < y && PressedY >= y - TILE_SIZE) {
				dir = SPUZ_DOWN;
			}
		}
		// Could be left or right of empty panel
		else if (PressedY > y && PressedY < y + TILE_SIZE) {
			// Right of empty panel
			if (PressedX > x + TILE_SIZE && PressedX <= x + TILE_SIZE * 2) {
				dir = SPUZ_LEFT;
			}
			else if (PressedX < x && PressedX >= x - TILE_SIZE) {
				dir = SPUZ_RIGHT;
			}
		}
	}

	SPUZ_Move(&p, dir);
}

void RenderPuzzle() {
	/* Clear the screen. */
	const SDL_Color black = {0, 0, 0};
	SDL_SetRenderDrawColor(Renderer, black.r, black.g, black.b, SDL_ALPHA_OPAQUE);
	if (SDL_RenderClear(Renderer) < 0) {
		Quit("Unable to clear the screen:\n%s", SDL_GetError());
	}

	/* Draw the board. */
	SDL_Rect dst, src;
	src.w = TILE_SIZE;
	src.h = TILE_SIZE;
	dst.w = TILE_SIZE;
	dst.h = TILE_SIZE;
	dst.y = 0;
	for (int j = 0; j < SPUZ_BOARD_H; j++, dst.y += TILE_SIZE) {
		dst.x = 0;
		for (int i = 0; i < SPUZ_BOARD_W; i++, dst.x += TILE_SIZE) {
			const int panel = p.panels[j][i] - 1;

			if (panel >= 0) {
				src.x = (panel % SPUZ_BOARD_W) * TILE_SIZE;
				src.y = (panel / SPUZ_BOARD_W) * TILE_SIZE;
				if (SDL_RenderCopy(Renderer, Picture, &src, &dst) < 0) {
					Quit("Unable to render copy a puzzle panel to the screen:\n%s", SDL_GetError());
				}
			}
			else {
				src.x = TILE_SIZE;
				src.y = TILE_SIZE;
				if (p.emptyX == 0) {
					src.x = 0;
				}
				else if (p.emptyX == SPUZ_BOARD_W - 1) {
					src.x = TILE_SIZE * 2;
				}

				if (p.emtpyY == 0) {
					src.y = 0;
				}
				else if (p.emtpyY == SPUZ_BOARD_H - 1) {
					src.y = TILE_SIZE * 2;
				}

				if (SDL_RenderCopy(Renderer, Arrows, &src, &dst) < 0) {
					Quit("Unable to render copy the empty puzzle panel to the screen:\n%s", SDL_GetError());
				}
			}
		}
	}

	SDL_RenderPresent(Renderer);
}

void RenderSolved(bool solved) {
	/* Clear the screen. */
	const SDL_Color black = {0, 0, 0}; 
	if (SDL_SetRenderDrawColor(Renderer, black.r, black.g, black.b, SDL_ALPHA_OPAQUE) < 0) {
		Quit("Failed to set render draw color:\n%s", SDL_GetError());
	}
	if (SDL_RenderClear(Renderer) < 0) {
		Quit("Unable to clear the screen:\n%s", SDL_GetError());
	}

	/* Draw the picture. */
	if (SDL_RenderCopy(Renderer, Picture, NULL, NULL) < 0) {
		Quit("Unable to render copy the picture to the screen:\n%s", SDL_GetError());
	}

	if (!solved) {
		SDL_Rect dst;
		dst.x = TILE_SIZE * 3;
		dst.y = TILE_SIZE * 3;
		dst.w = TILE_SIZE;
		dst.h = TILE_SIZE;
		if (SDL_RenderFillRect(Renderer, &dst) < 0) {
			Quit("Unable to render fill a rectangle:\n%s", SDL_GetError());
		}
	}

	SDL_RenderPresent(Renderer);
}

void Quit(const char* const format, ...) {
	if (Arrows) SDL_DestroyTexture(Arrows);
	Arrows = NULL;
	if (Picture) SDL_DestroyTexture(Picture);
	Picture = NULL;
	if (Renderer) SDL_DestroyRenderer(Renderer);
	Renderer = NULL;
	if (Window) SDL_DestroyWindow(Window);
	Window = NULL;
	if (SDL_WasInit(0)) SDL_Quit();
	
	if (format) {
		va_list args;

		va_start(args, format);
		const int size = vsnprintf(NULL, 0u, format, args) + 1;
		va_end(args);

		char* message = malloc(size);
		if (message != NULL) {
			va_start(args, format);
			vsnprintf(message, size, format, args);
			va_end(args);
			
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
			free(message);
		}

		exit(EXIT_FAILURE);
	}
	else {
		exit(EXIT_SUCCESS);
	}
}
