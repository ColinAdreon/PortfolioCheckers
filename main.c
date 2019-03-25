/*
 *	CHECKERS
 *	Colin C. Adreon
 *	The University of Alabama
 *	29 March 2019
 *
 *	Created for CS 201 - Data Structures and Algorithms
 *	Monica D. Anderson, PhD.
 *
 *	Main program function with integrated GUI
 *
 */

/*
	Board Layout Numbers

		black
	- @ - @ - @ - @		spaces 0-3 (left to right)
	@ - @ - @ - @ -		 4-7
	- @ - @ - @ - @		 8-11
	+ - + - + - + -		12-15
	- + - + - + - +		16-19
	O - O - O - O -		20-23
	- O - O - O - O		24-27
	O - O - O - O -		28-31
		white
*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "initFunctions.h"

int moveSelection(int, int);
int moveDirection(int, int, int, Square[]);
int jumpDirection(int, int, int, Square[]);
void movePiece(int, int, int, Square [], WINDOW *);

int main() {

	int i;
	int key			= 0;		// user input variable
	int csp			= 20;		// currently user selected piece
	int cpt			= 16;		// square currently pointed to
	int jmp 		= 0;		// square that was jumped
	int done		= 0;		// control
	int turn		= -1;		// control - white goes first
	int gameMode	= 1;		// control - 0 = menu, 1-2 = number of players, 3 = done
	int winsWhite	= 0;
	int winsBlack	= 0;
	uint32 moveable = 0xf00;
	uint32 AImove	= 0;		// check initFunctions.h for more details

	Square board[32];
	WINDOW * mainMenu;
	WINDOW * gameWind;
	Node * tree = NULL;			// minimax move-finding tree

	boardLayout(board);
	initscr();
	noecho();					// hides user input
	if (!has_colors()) {
		printf("\n\nThis program requires terminal color support.\n\n");
		return 0;
	}
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_WHITE, COLOR_BLUE);
	init_pair(4, COLOR_RED, COLOR_BLUE);
	init_pair(5, COLOR_WHITE, COLOR_GREEN);
	init_pair(6, COLOR_RED, COLOR_GREEN);
	init_pair(7, COLOR_BLACK, COLOR_YELLOW);
	init_pair(8, COLOR_RED, COLOR_YELLOW);
	attron(COLOR_PAIR(1));

	while(gameMode != 3) {		// program loop

		mainMenu = newwin(22, 72, 1, 4);
		gameMode = doMainMenu(mainMenu);

		while (gameMode == 1) { // human vs. computer

			gameWind = newwin(22, 72, 1, 4);

			mvwprintw(gameWind, 17, 39, "White:");
			mvwprintw(gameWind, 18, 39, "Red:");
			wattron  (gameWind, A_BOLD);
			mvwaddch (gameWind, 17, 46, numbers[winsWhite / 10]);
			waddch   (gameWind,			numbers[winsWhite % 10]);
			mvwaddch (gameWind, 18, 46, numbers[winsBlack / 10]);
			waddch   (gameWind, 		numbers[winsBlack % 10]);

			initGameBoard(gameWind);
			for (i = 0; i < 32; i++) draw(i, board, gameWind, 0);
			wrefresh(gameWind);

			while (!done) { // each game
				jmp = checkJumpsAll(turn, board);
				if (jmp) {
					moveable = jmp;
					wattron(gameWind, COLOR_PAIR(2));
					mvwprintw(gameWind, 16, 39, "You can jump!");
				}
				else {
					moveable = checkMovesAll(turn, board);
					wattron(gameWind, COLOR_PAIR(1));
					mvwprintw(gameWind, 16, 39, "             ");
				}
				if (!moveable) done = 1; // game ends if a player can't move
				else {

					wattron(gameWind, COLOR_PAIR(1));
					mvwprintw(gameWind, 15, 39, turnMsg[turn + 1]);
					key = 0;

					if (turn == -1) {
						do { // human turn loop
							draw(cpt, board, gameWind, 0);
							do { // handles movement
								draw(csp, board, gameWind, 0);
								csp = moveSelection(csp, key);
								draw(csp, board, gameWind, 1);
								key = wgetch(gameWind);
							} while ((key != 10 && key != KEY_ENTER) || !(moveable & (int)pwr(2, csp)));

							draw(csp, board, gameWind, 2);
							key = KEY_LEFT;

							do { // handles direction
								draw(cpt, board, gameWind, 0);
								if (jmp)
									cpt = jumpDirection(key, csp, cpt, board);
								else
									cpt = moveDirection(key, csp, cpt, board);
								draw(cpt, board, gameWind, ((jmp) ? 3 : 1));
								key = wgetch(gameWind);
							} while (key != 10 && key != KEY_ENTER && key != KEY_BACKSPACE);
						} while (key == KEY_BACKSPACE);

						movePiece(csp, cpt, jmp, board, gameWind);
						draw(csp, board, gameWind, 0);
						tree = humanMove(csp, cpt, jmp, board);
						//printr(tree->info);
					}

					else { // AI turn loop
						AImove = chooseMove(tree);
						//printr(AImove);
						csp = (AImove / 0x200) & 0x1F;
						cpt = (AImove / 0x10) & 0x1F;
						jmp = jumpedSquare(csp, cpt);
						movePiece(csp, cpt, jmp, board, gameWind);
						draw(csp, board, gameWind, 0);
					}

					if (!(jmp && checkJumpsAll(turn, board) & pwr(2, cpt)))
						turn *= -1;
					csp = cpt;
					wrefresh(gameWind);
				}
			}

			delwin(gameWind);
			if (turn == 1) winsWhite++;
			else winsBlack++;
			gameWind = newwin(8, 32, 8, 10);
			if (initEndScreen(gameWind)) gameMode = 0;
			else done = 0;
			boardLayout(board);
			delwin(gameWind);

			turn = -1;

		}

		while (gameMode == 2) {	// two humans

			gameWind = newwin(22, 72, 1, 4);

			mvwprintw(gameWind, 17, 39, "White:");
			mvwprintw(gameWind, 18, 39, "Red:");
			wattron  (gameWind, A_BOLD);
			mvwaddch (gameWind, 17, 46, numbers[winsWhite / 10]);
			waddch   (gameWind,			numbers[winsWhite % 10]);
			mvwaddch (gameWind, 18, 46, numbers[winsBlack / 10]);
			waddch   (gameWind, 		numbers[winsBlack % 10]);

			initGameBoard(gameWind);
			for (i = 0; i < 32; i++) draw(i, board, gameWind, 0);
			wrefresh(gameWind);

			while (!done) { // each game
				jmp = checkJumpsAll(turn, board);
				if (jmp) {
					moveable = jmp;
					wattron(gameWind, COLOR_PAIR(2));
					mvwprintw(gameWind, 16, 39, "You can jump!");
				}
				else {
					moveable = checkMovesAll(turn, board);
					wattron(gameWind, COLOR_PAIR(1));
					mvwprintw(gameWind, 16, 39, "             ");
				}
				if (!moveable) done = 1; // game ends if a player can't move
				else {
					wattron(gameWind, COLOR_PAIR(1));
					mvwprintw(gameWind, 15, 39, turnMsg[turn + 1]);
					key = 0;
					do { // turn loop
						draw(cpt, board, gameWind, 0);
						do { // handles movement
							draw(csp, board, gameWind, 0);
							csp = moveSelection(csp, key);
							draw(csp, board, gameWind, 1);
							key = wgetch(gameWind);
						} while ((key != 10 && key != KEY_ENTER) || !(moveable & (int)pwr(2, csp)));

						draw(csp, board, gameWind, 2);
						// wgetch(gameWind);
						// if (jmp) cpt = jumpDirection(KEY_LEFT, csp, 32, board);
						// else 	 cpt = moveDirection(KEY_LEFT, csp, 32, board);
						key = KEY_LEFT;

						do { // handles direction
							draw(cpt, board, gameWind, 0);
							if (jmp)
								cpt = jumpDirection(key, csp, cpt, board);
							else
								cpt = moveDirection(key, csp, cpt, board);
							draw(cpt, board, gameWind, ((jmp) ? 3 : 1));
							key = wgetch(gameWind);
						} while (key != 10 && key != KEY_ENTER && key != KEY_BACKSPACE);
					} while (key == KEY_BACKSPACE);

					movePiece(csp, cpt, jmp, board, gameWind);
					draw(csp, board, gameWind, 0);
					csp = cpt;
					if (!(jmp && checkJumpsAll(turn, board) & pwr(2, cpt)))
						turn *= -1;
					wrefresh(gameWind);
				}
			}

			delwin(gameWind);
			if (turn == 1) winsWhite++;
			else winsBlack++;
			gameWind = newwin(8, 32, 8, 10);
			if (initEndScreen(gameWind)) gameMode = 0;
			else done = 0;
			boardLayout(board);
			delwin(gameWind);
		}
	}
	endwin();
	return 0;
}

int moveSelection(int piece, int key) {
	int ndir;
	switch (key) {
		case KEY_UP:
			if (piece > 3)  ndir = piece - 4;
			break;
		case KEY_DOWN:
			if (piece < 28) ndir = piece + 4;
			break;
		case KEY_LEFT:
			if (piece % 4)  ndir = piece - 1;
			break;
		case KEY_RIGHT:
			if (piece % 4 != 3) ndir = piece + 1;
		default:
			break;
	}
	return ndir;
}

int moveDirection(int key, int piece, int old, Square board[32]) {
	if (board[piece].occ == 1) {
		if (board[piece].se && (!board[piece].sw || board[piece].sw->occ))
			return board[piece].se->sqNum;
		if (board[piece].sw && (!board[piece].se || board[piece].se->occ))
			return board[piece].sw->sqNum;
		if (key == KEY_LEFT || key == KEY_RIGHT) {
			if (old == board[piece].se->sqNum && !board[piece].sw->occ)
				return board[piece].sw->sqNum;
			if (old == board[piece].sw->sqNum && !board[piece].se->occ)
				return board[piece].se->sqNum;
		}
		else return old;
	}
	else if (board[piece].occ == -1) {
		if (board[piece].ne && (!board[piece].nw || board[piece].nw->occ))
			return board[piece].ne->sqNum;
		if (board[piece].nw && (!board[piece].ne || board[piece].ne->occ))
			return board[piece].nw->sqNum;
		if (key == KEY_LEFT || key == KEY_RIGHT) {
			if (old == board[piece].ne->sqNum && !board[piece].nw->occ)
				return board[piece].nw->sqNum;
			if (old == board[piece].nw->sqNum && !board[piece].ne->occ)
				return board[piece].ne->sqNum;
		}
		else return old;
	}
	switch (key) {
		case KEY_LEFT:
			if 		(board[piece].ne && old == board[piece].ne->sqNum) {
				if  (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				if  (board[piece].sw && !board[piece].sw->occ) return board[piece].sw->sqNum;
				if  (board[piece].se && !board[piece].se->occ) return board[piece].se->sqNum;
				else				 return old;
			}
			else if (board[piece].se && old == board[piece].se->sqNum) {
				if  (board[piece].ne && !board[piece].ne->occ) return board[piece].ne->sqNum;
				if  (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				if  (board[piece].sw && !board[piece].sw->occ) return board[piece].sw->sqNum;
				else				 return old;
			}
			else if (board[piece].sw && old == board[piece].sw->sqNum) {
				if  (board[piece].se && !board[piece].se->occ) return board[piece].se->sqNum;
				if  (board[piece].ne && !board[piece].ne->occ) return board[piece].ne->sqNum;
				if  (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				else				 return old;
			}
			else {
				if (board[piece].sw && !board[piece].sw->occ) return board[piece].sw->sqNum;
				if (board[piece].se && !board[piece].se->occ) return board[piece].se->sqNum;
				if (board[piece].ne && !board[piece].ne->occ) return board[piece].ne->sqNum;
				if (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				else				 return old;
			}
			break;
		case KEY_RIGHT:
			if 		(board[piece].ne && old == board[piece].ne->sqNum) {
				if  (board[piece].se && !board[piece].se->occ) return board[piece].se->sqNum;
				if  (board[piece].sw && !board[piece].sw->occ) return board[piece].sw->sqNum;
				if  (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				else				 return old;
			}
			else if (board[piece].se && old == board[piece].se->sqNum) {
				if  (board[piece].sw && !board[piece].sw->occ) return board[piece].sw->sqNum;
				if  (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				if  (board[piece].ne && !board[piece].ne->occ) return board[piece].ne->sqNum;
				else				 return old;
			}
			else if (board[piece].sw && old == board[piece].sw->sqNum) {
				if  (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				if  (board[piece].ne && !board[piece].ne->occ) return board[piece].ne->sqNum;
				if  (board[piece].se && !board[piece].se->occ) return board[piece].se->sqNum;
				else				 return old;
			}
			else {
				if (board[piece].ne && !board[piece].ne->occ) return board[piece].ne->sqNum;
				if (board[piece].se && !board[piece].se->occ) return board[piece].se->sqNum;
				if (board[piece].sw && !board[piece].sw->occ) return board[piece].sw->sqNum;
				if (board[piece].nw && !board[piece].nw->occ) return board[piece].nw->sqNum;
				else				 return old;
			}
			break;
		default:
			return old;
	}
}

int jumpDirection(int key, int piece, int old, Square board[32]) {
	if (key == KEY_BACKSPACE) return piece;
	if (board[piece].occ == 1) { // this needs checking for legality
		if (board[piece].se && !(board[piece].sw && board[piece].sw->sw))
			return board[piece].se->se->sqNum;
		if (board[piece].sw && !(board[piece].se && board[piece].se->se))
			return board[piece].sw->sw->sqNum;
		if (key == KEY_LEFT || key == KEY_RIGHT) {
			if (old == board[piece].se->se->sqNum) {
				if (board[piece].sw->occ < 0 && !board[piece].sw->sw->occ)
					return board[piece].sw->sw->sqNum;
				else return board[piece].se->se->sqNum;
			}
			else {//if (old == board[piece].sw->sw->sqNum) {
				if (board[piece].se->occ < 0 && !board[piece].se->se->occ)
					return board[piece].se->se->sqNum;
				else return board[piece].sw->sw->sqNum;
			}
		}
		else return old;
	}
	else if (board[piece].occ == -1) {
		if (board[piece].ne && !(board[piece].nw && board[piece].nw->nw))
			return board[piece].ne->ne->sqNum;
		if (board[piece].nw && !(board[piece].ne && board[piece].ne->ne))
			return board[piece].nw->nw->sqNum;
		if (key == KEY_LEFT || key == KEY_RIGHT) {
			if (old == board[piece].ne->ne->sqNum) {
				if (board[piece].nw->occ > 0 && !board[piece].nw->nw->occ)
					return board[piece].nw->nw->sqNum;
				else return board[piece].ne->ne->sqNum;
			}
			else {//if (old == board[piece].nw->nw->sqNum) {
				if (board[piece].ne->occ > 0 && !board[piece].ne->ne->occ)
					return board[piece].ne->ne->sqNum;
				else return board[piece].nw->nw->sqNum;
			}
		}
		else return old;
	}
	switch (key) {
		case KEY_LEFT:
			if 		(board[piece].ne && board[piece].ne->ne && old == board[piece].ne->ne->sqNum) {
				if  (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				if  (board[piece].sw && board[piece].sw->sw && !board[piece].sw->sw->occ && board[piece].occ * board[piece].sw->occ == -2) return board[piece].sw->sw->sqNum;
				if  (board[piece].se && board[piece].se->se && !board[piece].se->se->occ && board[piece].occ * board[piece].se->occ == -2) return board[piece].se->se->sqNum;
				else				 return old;
			}
			else if (board[piece].se && board[piece].se->se && old == board[piece].se->se->sqNum) {
				if  (board[piece].ne && board[piece].ne->ne && !board[piece].ne->ne->occ && board[piece].occ * board[piece].ne->occ == -2) return board[piece].ne->ne->sqNum;
				if  (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				if  (board[piece].sw && board[piece].sw->sw && !board[piece].sw->sw->occ && board[piece].occ * board[piece].sw->occ == -2) return board[piece].sw->sw->sqNum;
				else				 return old;
			}
			else if (board[piece].sw && board[piece].sw->sw && old == board[piece].sw->sw->sqNum) {
				if  (board[piece].se && board[piece].se->se && !board[piece].se->se->occ && board[piece].occ * board[piece].se->occ == -2) return board[piece].se->se->sqNum;
				if  (board[piece].ne && board[piece].ne->ne && !board[piece].ne->ne->occ && board[piece].occ * board[piece].ne->occ == -2) return board[piece].ne->ne->sqNum;
				if  (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				else				 return old;
			}
			else {
				if (board[piece].sw && board[piece].sw->sw && !board[piece].sw->sw->occ && board[piece].occ * board[piece].sw->occ == -2) return board[piece].sw->sw->sqNum;
				if (board[piece].se && board[piece].se->se && !board[piece].se->se->occ && board[piece].occ * board[piece].se->occ == -2) return board[piece].se->se->sqNum;
				if (board[piece].ne && board[piece].ne->ne && !board[piece].ne->ne->occ && board[piece].occ * board[piece].ne->occ == -2) return board[piece].ne->ne->sqNum;
				if (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				else				 return old;
			}
			break;
		case KEY_RIGHT:
			if 		(board[piece].ne && board[piece].ne->ne && old == board[piece].ne->ne->sqNum) {
				if  (board[piece].se && board[piece].se->se && !board[piece].se->se->occ && board[piece].occ * board[piece].se->occ == -2) return board[piece].se->se->sqNum;
				if  (board[piece].sw && board[piece].sw->sw && !board[piece].sw->sw->occ && board[piece].occ * board[piece].sw->occ == -2) return board[piece].sw->sw->sqNum;
				if  (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				else				 return old;
			}
			else if (board[piece].se && board[piece].se->se && old == board[piece].se->se->sqNum) {
				if  (board[piece].sw && board[piece].sw->sw && !board[piece].sw->sw->occ && board[piece].occ * board[piece].sw->occ == -2) return board[piece].sw->sw->sqNum;
				if  (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				if  (board[piece].ne && board[piece].ne->ne && !board[piece].ne->ne->occ && board[piece].occ * board[piece].ne->occ == -2) return board[piece].ne->ne->sqNum;
				else				 return old;
			}
			else if (board[piece].sw && board[piece].sw->sw && old == board[piece].sw->sw->sqNum) {
				if  (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				if  (board[piece].ne && board[piece].ne->ne && !board[piece].ne->ne->occ && board[piece].occ * board[piece].ne->occ == -2) return board[piece].ne->ne->sqNum;
				if  (board[piece].se && board[piece].se->se && !board[piece].se->se->occ && board[piece].occ * board[piece].se->occ == -2) return board[piece].se->se->sqNum;
				else				 return old;
			}
			else {
				if (board[piece].ne && board[piece].ne->ne && !board[piece].ne->ne->occ && board[piece].occ * board[piece].ne->occ == -2) return board[piece].ne->ne->sqNum;
				if (board[piece].se && board[piece].se->se && !board[piece].se->se->occ && board[piece].occ * board[piece].se->occ == -2) return board[piece].se->se->sqNum;
				if (board[piece].sw && board[piece].sw->sw && !board[piece].sw->sw->occ && board[piece].occ * board[piece].sw->occ == -2) return board[piece].sw->sw->sqNum;
				if (board[piece].nw && board[piece].nw->nw && !board[piece].nw->nw->occ && board[piece].occ * board[piece].nw->occ == -2) return board[piece].nw->nw->sqNum;
				else				 return old;
			}
			break;
		default:
			return old;
	}
}

void movePiece(int src, int dst, int jmp, Square board[32], WINDOW * gameWind) {

	if (!jmp) { // we don't have to worry about square 0, since it's an edge
		board[dst].occ = board[src].occ;
		board[src].occ = 0;
		draw(src, board, gameWind, 0);
	}
	else {
		board[dst].occ = board[src].occ;
		board[src].occ = 0;
		if      (board[dst].ne == board[src].sw) {
			board[dst].ne->occ = 0;
			draw(board[dst].ne->sqNum, board, gameWind, 0);
		}
		else if (board[dst].se == board[src].nw) {
			board[dst].se->occ = 0;
			draw(board[dst].se->sqNum, board, gameWind, 0);
		}
		else if (board[dst].sw == board[src].ne) {
			board[dst].sw->occ = 0;
			draw(board[dst].sw->sqNum, board, gameWind, 0);
		}
		else {
			board[dst].nw->occ = 0;
			draw(board[dst].nw->sqNum, board, gameWind, 0);
		}
	}
	if 		(dst > 27 && board[dst].occ ==  1) board[dst].occ =  2; // crown
	else if (dst <  4 && board[dst].occ == -1) board[dst].occ = -2; // kings
	draw(dst, board, gameWind, 0);
	return;
}
