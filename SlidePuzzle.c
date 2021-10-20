#include "SlidePuzzle.h"
#include <stdio.h>
#include <stdlib.h>

void SPUZ_Init(SPUZ_Board* p) {
	for (size_t j = 0; j < SPUZ_BOARD_H; j++) {
		for (size_t i = 0; i < SPUZ_BOARD_W; i++) {
			p->panels[j][i] = i + (SPUZ_BOARD_W * j) + 1;
		}
	}
	p->panels[SPUZ_BOARD_H - 1][SPUZ_BOARD_W - 1] = 0;

	p->emptyX = SPUZ_BOARD_W - 1;
	p->emtpyY = SPUZ_BOARD_H - 1;

	return;
}

void SPUZ_Permute(SPUZ_Board* p, unsigned int seed) {
	/* Reading the "Solvability" section in the "Fifteen puzzle" article on
	 * Wikipedia, I noticed one can use the criteria for solvability to
	 * easily randomize the puzzle, and guarantee solvability. Those
	 * criteria are used in the code of this function. */

	/* Swap panels an even number of times. 100 was chosen arbitrarily, it
	 * seems good enough. */
	srand(seed);
	int a = rand() % (SPUZ_BOARD_H * SPUZ_BOARD_W);
	for (int i = 0; i < 100; i++) {
		int b;
		while ((b = rand() % (SPUZ_BOARD_H * SPUZ_BOARD_W)) == a);
		int tmp = p->panels[a / SPUZ_BOARD_W][a % SPUZ_BOARD_W];
		p->panels[a / SPUZ_BOARD_W][a % SPUZ_BOARD_W] = p->panels[b / SPUZ_BOARD_W][b % SPUZ_BOARD_W];
		p->panels[b / SPUZ_BOARD_W][b % SPUZ_BOARD_W] = tmp;
	}

	/* Now if the parity of the taxicab distance the empty square moved is
	 * odd, choose two squares that aren't empty and exchange them, making
	 * the parities match up, and the puzzle solvable. */

	/* First find the empty square, resetting the coordinates in the SPUZ_Board
	 * struct. */
	for (size_t j = 0; j < SPUZ_BOARD_H; j++) {
		for (size_t i = 0; i < SPUZ_BOARD_W; i++) {
			if (p->panels[j][i] == 0) {
				p->emptyX = i;
				p->emtpyY = j;
				goto FOUND_EMPTY;
			}
		}
	}
FOUND_EMPTY:

	/* Next check the parity of the taxicab distance the empty square
	 * moved, and if odd, permute two non-empty panels. */
	int dx = p->emptyX - SPUZ_BOARD_W - 1;
	if (dx < 0) dx = -dx;
	int dy = p->emtpyY - SPUZ_BOARD_H - 1;
	if (dy < 0) dy = -dy;

	if ((dx + dy) % 2) {
		a = rand() % 15;
		int b;
		while ((b = rand() % (SPUZ_BOARD_H * SPUZ_BOARD_W)) == a);
		int c = p->emptyX + 4 * p->emtpyY;
		if (a >= c) a++;
		if (b >= c) b++;
		int tmp = p->panels[a / SPUZ_BOARD_W][a % SPUZ_BOARD_W];
		p->panels[a / SPUZ_BOARD_W][a % SPUZ_BOARD_W] = p->panels[b / SPUZ_BOARD_W][b % SPUZ_BOARD_W];
		p->panels[b / SPUZ_BOARD_W][b % SPUZ_BOARD_W] = tmp;
	}

	return;
}

void SPUZ_Print(SPUZ_Board* p) {
	for (size_t j = 0; j < SPUZ_BOARD_H; j++) {
		printf("|");
		for (size_t i = 0; i < SPUZ_BOARD_W; i++) {
			printf("%2d|", p->panels[j][i]);
		}
		printf("\n");
	}
	printf("Is %sSolved\n", SPUZ_Solved(p) ? "" : "Not ");

	return;
}

bool SPUZ_Move(SPUZ_Board* p, SPUZ_Direction dir) {
	bool moved = false;

	switch (dir) {
	case SPUZ_UP:
		if (p->emtpyY == SPUZ_BOARD_H - 1) {
			break;
		}
		else {
			p->panels[p->emtpyY][p->emptyX] = p->panels[p->emtpyY + 1][p->emptyX];
			p->emtpyY++;
			moved = true;
		}
		break;
	case SPUZ_DOWN:
		if (p->emtpyY == 0) {
			break;
		}
		else {
			p->panels[p->emtpyY][p->emptyX] = p->panels[p->emtpyY - 1][p->emptyX];
			p->emtpyY--;
			moved = true;
		}
		break;
	case SPUZ_LEFT:
		if (p->emptyX == SPUZ_BOARD_W - 1) {
			break;
		}
		else {
			p->panels[p->emtpyY][p->emptyX] = p->panels[p->emtpyY][p->emptyX + 1];
			p->emptyX++;
			moved = true;
		}
		break;
	case SPUZ_RIGHT:
		if (p->emptyX == 0) {
			break;
		}
		else {
			p->panels[p->emtpyY][p->emptyX] = p->panels[p->emtpyY][p->emptyX - 1];
			p->emptyX--;
			moved = true;
		}
		break;
	default:
		break;
	}
	if (moved) {
		p->panels[p->emtpyY][p->emptyX] = 0;
	}

	return moved;
}

bool SPUZ_Solved(SPUZ_Board* p) {
	bool solved = true;

	if (p->panels[SPUZ_BOARD_H - 1][SPUZ_BOARD_W - 1] != 0) {
		solved = false;
	}
	else {
		for (size_t i = 0; i < (SPUZ_BOARD_W * SPUZ_BOARD_H) - 2; i++) {
			if (p->panels[(i + 1) / SPUZ_BOARD_W][(i + 1) % SPUZ_BOARD_W] < p->panels[i / SPUZ_BOARD_W][i % SPUZ_BOARD_W]) {
				solved = false;
				break;
			}
		}
	}

	return solved;
}
