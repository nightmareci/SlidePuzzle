#pragma once

#include <stdbool.h>

#define SPUZ_BOARD_H 4
#define SPUZ_BOARD_W 4

typedef struct SPUZ_Board {
	int panels[SPUZ_BOARD_H][SPUZ_BOARD_W]; /* Contents of the slide puzzle. */
	int emptyX; /* Horizontal position of the empty square. */
	int emtpyY; /* Vertical position of the empty square. */
} SPUZ_Board;

typedef enum SPUZ_Direction {
	SPUZ_UP,
	SPUZ_DOWN,
	SPUZ_LEFT,
	SPUZ_RIGHT,
	SPUZ_INVALIDDIRECTION
} SPUZ_Direction;

void SPUZ_Init(SPUZ_Board* p);
void SPUZ_Permute(SPUZ_Board* p, unsigned int seed);
void SPUZ_Print(SPUZ_Board* p);
bool SPUZ_Move(SPUZ_Board* p, SPUZ_Direction dir);
bool SPUZ_Solved(SPUZ_Board* p);