#ifndef NULL
#define NULL ((void *)0)
#endif

// I realize all the code says black/white but the visuals are red/white.
// It took an embarrasingly long time to figure out how to make black
// pieces show up on a black background before I gave up.

// Also, I highly recommend reviewing binary and bitmath if you are not
// entirely familiar with the subject. For most of this project, any variable
// of type "uint32" should be thought of as an array of binary digits,
// rather than an integer in and of itself

typedef __uint32_t uint32; // 'cuz underscores are too much work

const int DIFF = 4; // depth of minimax tree, works up to 7
					// I recommend no higher than 4, otherwise the tree gets
					// very large and you running out of heap space


const int xPositions[32]	 = {1, 3, 5, 7, 0, 2, 4, 6,
								1, 3, 5, 7, 0, 2, 4, 6,
								1, 3, 5, 7, 0, 2, 4, 6,
								1, 3, 5, 7, 0, 2, 4, 6 };
const int yPositions[32]	 = {0, 0, 0, 0, 1, 1, 1, 1,
								2, 2, 2, 2, 3, 3, 3, 3,
								4, 4, 4, 4, 5, 5, 5, 5,
								6, 6, 6, 6, 7, 7, 7, 7 };

const char menuLabels[3][12] = {"One Player\0", "Two Players\0", "Exit Game\0"};
const char turnMsg[3][14] 	 = {"White's turn!\0", "\0", "Red's turn!  \0"};
const char numbers[10] = "0123456789";
const uint32 ne[32] = {		0,			0,			0,			0,
							1,			2,			4,			8,
					 	 0x20,		 0x40,		 0x80,		 	0,
						0x100,		0x200,		0x400,		0x800,
				   	   0x2000,	   0x4000,	   0x8000,			0,
				  	  0x10000,	  0x20000,	  0x40000,	  0x80000,
				 	 0x200000,	 0x400000,	 0x800000,			0,
					0x1000000,	0x2000000,	0x4000000,	0x8000000
};
const uint32 se[32] = {	 0x20,		 0x40,		 0x80,			0,
						0x100,		0x200,		0x400,		0x800,
					   0x2000,	   0x4000,	   0x8000,			0,
					  0x10000,	  0x20000,	  0x40000,	  0x80000,
					 0x200000,	 0x400000,	 0x800000,			0,
				   	0x1000000,	0x2000000,	0x4000000,	0x8000000,
				   0x20000000, 0x40000000, 0x80000000,			0,
				   			0,			0,			0,			0
};
const uint32 sw[32] = {	 0x10,		 0x20,		 0x40,		 0x80,
							0,		0x100,		0x200,		0x400,
					   0x1000,	   0x2000,	   0x4000,	   0x8000,
					   		0,	  0x10000,	  0x20000,	  0x40000,
					 0x100000,	 0x200000,	 0x400000,	 0x800000,
							0,	0x1000000,	0x2000000,	0x4000000,
				   0x10000000, 0x20000000, 0x40000000, 0x80000000,
				   			0,			0,			0,			0
};
const uint32 nw[32] = {		0,			0,			0,			0,
							0,			1,			2,			4,
						 0x10,		 0x20,		 0x40,		 0x80,
							0,		0x100,		0x200,		0x400,
					   0x1000,	   0x2000,	   0x4000,	   0x8000,
							0,	  0x10000,	  0x20000,	  0x40000,
					 0x100000,	 0x200000,	 0x400000, 	 0x800000,
							0,	0x1000000,	0x2000000,	0x4000000
};


typedef struct node {
	uint32 white;		// remember, this is an array, not a number
	uint32 black;		// "
	uint32 kings;		// "
	uint32 info;		// 1-bit crowning, 5-bit jumped (0 cannot be jumped),
						// 5-bit src, 5-bit dst, 1-bit turn, 3-bit level
	// I realize this may seem inefficient, but it's necessary to conserve
	// as much space as possible, since the tree will be very large

	int heur;			// Value used to compare board states
	struct node *sib;	// list of right siblings
	struct node *chl;	// left child
} Node;

//	+			graph structure
//	|
//	+-----------+-----------+-----------+---------------+
//	|			|			|			|				|
//	+---+---+	+---+---+	+---+---+	+---+---+---+	+---+
//	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|


typedef struct square {
	int occ;		// 0 = empty, 1 = pawn, 2 = king, + black, - white
	struct square *ne;	// From white's perspective
	struct square *se;	// north is toward black
	struct square *sw;	// east is to the right
	struct square *nw;
	int sqNum;
} Square;

uint32 pwr(int, int);

int lg(uint32);

// ===== Graphics ===== //

int doMainMenu(WINDOW * mainMenu) {
	int gameMode = 1;
	int i, key;
	refresh();
	wattron(mainMenu, COLOR_PAIR(1));
	box(mainMenu, 0, 0);
	wrefresh(mainMenu);
	keypad(mainMenu, true);
	mvwprintw(mainMenu, 1, 1, "CHECKERS");
	do { 					// Main Menu
		for (i = 1; i < 4; i++) {
			if (i == gameMode) wattron(mainMenu, A_REVERSE);
			mvwprintw(mainMenu, i + 2, 6, menuLabels[i-1]);
			wattroff(mainMenu, A_REVERSE);
		}
		mvwprintw(mainMenu, 20, 69, " ");
		wrefresh(mainMenu);
		key = wgetch(mainMenu);
		switch (key) {
			case KEY_UP:
				if (gameMode > 1) gameMode--;
				break;
			case KEY_DOWN:
				if (gameMode < 3) gameMode++;
				break;
			default:
				break;
		}
	} while(key != 10 && key != KEY_ENTER); // 10 = enter key
	return gameMode;
}

// Visually setup board
void initGameBoard(WINDOW * gameWind) {
	int i;
	refresh();
	wattron(gameWind, COLOR_PAIR(1));
	box(gameWind, 0, 0);
	wrefresh(gameWind);
	keypad(gameWind, true);
	wattron(gameWind, A_REVERSE);
	for (i = 3; i < 19; i++) {
		mvwprintw(gameWind, i, 6, "                                ");
	}
	wattroff(gameWind, A_REVERSE);
	for (i = 0; i < 32; i++) {
		mvwprintw(gameWind, 2 * yPositions[i] + 3, 4 * xPositions[i] + 6, "    ");
		mvwprintw(gameWind, 2 * yPositions[i] + 4, 4 * xPositions[i] + 6, "    ");
	}
	wattroff (gameWind, A_BOLD);
	mvwprintw(gameWind,  3, 39, "Move with the arrow keys.");
	mvwprintw(gameWind,  5, 39, "Press Enter to select a piece.");
	mvwprintw(gameWind,  7, 39, "Press Backspace to deselect.");
	mvwprintw(gameWind,  9, 39, "Once a piece is selected, use");
	mvwprintw(gameWind, 10, 39, "the arrow keys to change");
	mvwprintw(gameWind, 11, 39, "movement direction.");
	mvwprintw(gameWind, 13, 39, "Press Enter to confirm move");
	return;
}

// The box at the end that asks to play again
int initEndScreen(WINDOW * hwnd) {
	int sel = 0;
	int key = 0;
	refresh();
	wattroff(hwnd, A_BOLD);
	wattron(hwnd, COLOR_PAIR(1));
	box(hwnd, 0, 0);
	wrefresh(hwnd);
	keypad(hwnd, true);
	mvwprintw(hwnd, 2, 10, "Play again?");
	do {
		if (!sel) wattron(hwnd, A_REVERSE);
		mvwprintw(hwnd, 5, 8, "Yes");
		wattroff(hwnd, A_REVERSE);
		if (sel)  wattron(hwnd, A_REVERSE);
		mvwprintw(hwnd, 5, 22, "No");
		wmove(hwnd, 1, 1);
		wattroff(hwnd, A_REVERSE);
		wrefresh(hwnd);

		key = wgetch(hwnd);
		switch (key) {
			case KEY_LEFT:
				if (sel)  sel = 0;
				break;
			case KEY_RIGHT:
				if (!sel) sel = 1;
				break;
			default:
				break;
		}
	} while(key != 10 && key != KEY_ENTER); // CR vs LF
	return sel;
}

void draw(int piece, Square b[32], WINDOW * hwnd, int state) {
	if (b[piece].occ > 0) wattron(hwnd, COLOR_PAIR(2 * state + 2));
	else				  wattron(hwnd, COLOR_PAIR(2 * state + 1));

	wattron(hwnd, A_BOLD);

	if (b[piece].occ % 2) {
		mvwaddch(hwnd, 2 * yPositions[piece] + 3, 4 * xPositions[piece] + 6, ACS_ULCORNER);
		waddch(hwnd, ACS_HLINE);
		waddch(hwnd, ACS_HLINE);
		waddch(hwnd, ACS_URCORNER);
		mvwaddch(hwnd, 2 * yPositions[piece] + 4, 4 * xPositions[piece] + 6, ACS_LLCORNER);
		waddch(hwnd, ACS_HLINE);
		waddch(hwnd, ACS_HLINE);
		waddch(hwnd, ACS_LRCORNER);
	}
	else if (b[piece].occ) {
		mvwaddch(hwnd, 2 * yPositions[piece] + 3, 4 * xPositions[piece] + 6, ACS_LTEE);
		waddch(hwnd, ACS_BTEE);
		waddch(hwnd, ACS_BTEE);
		waddch(hwnd, ACS_RTEE);
		mvwaddch(hwnd, 2 * yPositions[piece] + 4, 4 * xPositions[piece] + 6, ACS_LLCORNER);
		waddch(hwnd, ACS_HLINE);
		waddch(hwnd, ACS_HLINE);
		waddch(hwnd, ACS_LRCORNER);
	}
	else {
		mvwprintw(hwnd, 2 * yPositions[piece] + 3, 4 * xPositions[piece] + 6, "    ");
		mvwprintw(hwnd, 2 * yPositions[piece] + 4, 4 * xPositions[piece] + 6, "    ");
	}

	wmove(hwnd, 1, 1);
	return;
}

// ===== Gameplay ===== //

// Remeber: it's an array, not a number
uint32 checkJumpsAll(int color, Square board[32]) {
	uint32 m = 0; // I know it's weird, just treat it like a bool array
	int i;
	if (color == 1) {
		for (i = 0; i < 32; i++) {
			if (board[i].occ == 2)	{		// kings jump all directions
				if ((board[i].ne && board[i].ne->ne && board[i].ne->occ < 0 && !board[i].ne->ne->occ) ||
					(board[i].se && board[i].se->se && board[i].se->occ < 0 && !board[i].se->se->occ) ||
					(board[i].sw && board[i].sw->sw && board[i].sw->occ < 0 && !board[i].sw->sw->occ) ||
					(board[i].nw && board[i].nw->nw && board[i].nw->occ < 0 && !board[i].nw->nw->occ))
						m += pwr(2, i);
			}
			else if (board[i].occ == 1) {	// black jumps south
				if ((board[i].se && board[i].se->se && board[i].se->occ < 0 && !board[i].se->se->occ) ||
					(board[i].sw && board[i].sw->sw && board[i].sw->occ < 0 && !board[i].sw->sw->occ))
						m += pwr(2, i);
			}
		}
	}
	else if (color == -1) {
		for (i = 0; i < 32; i++) {
			if (board[i].occ == -2) {		// kings jump all directions
				if ((board[i].ne && board[i].ne->ne && board[i].ne->occ > 0 && !board[i].ne->ne->occ) ||
					(board[i].se && board[i].se->se && board[i].se->occ > 0 && !board[i].se->se->occ) ||
					(board[i].sw && board[i].sw->sw && board[i].sw->occ > 0 && !board[i].sw->sw->occ) ||
					(board[i].nw && board[i].nw->nw && board[i].nw->occ > 0 && !board[i].nw->nw->occ))
						m += pwr(2, i);
			}
			else if (board[i].occ == -1) {	// white jumps north
				if ((board[i].ne && board[i].ne->ne && board[i].ne->occ > 0 && !board[i].ne->ne->occ) ||
					(board[i].nw && board[i].nw->nw && board[i].nw->occ > 0 && !board[i].nw->nw->occ))
						m += pwr(2, i);
			}
		}
	}
	return m;
}

uint32 checkMovesAll(int color, Square board[32]) {
	uint32 m = 0;
	int i;
	if (color == 1) {
		for (i = 0; i < 32; i++) {
			if (board[i].occ == 2) {		// kings move all directions
				if ((board[i].ne && !board[i].ne->occ) ||
					(board[i].se && !board[i].se->occ) ||
					(board[i].sw && !board[i].sw->occ) ||
					(board[i].nw && !board[i].nw->occ))	m += pwr(2, i);
			}
			else if (board[i].occ == 1) {	// black moves south
				if ((board[i].se && !board[i].se->occ) ||
					(board[i].sw && !board[i].sw->occ))	m += pwr(2, i);
			}
		}
	}
	else if (color == -1) {
		for (i = 0; i < 32; i++) {
			if (board[i].occ == -2) {		// kings move all directions
				if ((board[i].ne && !board[i].ne->occ) ||
					(board[i].se && !board[i].se->occ) ||
					(board[i].sw && !board[i].sw->occ) ||
					(board[i].nw && !board[i].nw->occ))	m += pwr(2, i);
			}
			else if (board[i].occ == -1) {	// white moves north
				if ((board[i].ne && !board[i].ne->occ) ||
					(board[i].nw && !board[i].nw->occ))	m += pwr(2, i);
			}
		}
	}
	return m;
}

// Logically setup board
void boardLayout(Square b[32]) {
	b[ 0].occ = 1;
	b[ 0].ne  = NULL;
	b[ 0].se  = &b[5];
	b[ 0].sw  = &b[4];
	b[ 0].nw  = NULL;

	b[ 1].occ = 1;
	b[ 1].ne  = NULL;
	b[ 1].se  = &b[6];
	b[ 1].sw  = &b[5];
	b[ 1].nw  = NULL;

	b[ 2].occ = 1;
	b[ 2].ne  = NULL;
	b[ 2].se  = &b[7];
	b[ 2].sw  = &b[6];
	b[ 2].nw  = NULL;

	b[ 3].occ = 1;
	b[ 3].ne  = NULL;
	b[ 3].se  = NULL;
	b[ 3].sw  = &b[7];
	b[ 3].nw  = NULL;



	b[ 4].occ = 1;
	b[ 4].ne  = &b[0];
	b[ 4].se  = &b[8];
	b[ 4].sw  = NULL;
	b[ 4].nw  = NULL;

	b[ 5].occ = 1;
	b[ 5].ne  = &b[1];
	b[ 5].se  = &b[9];
	b[ 5].sw  = &b[8];
	b[ 5].nw  = &b[0];

	b[ 6].occ = 1;
	b[ 6].ne  = &b[2];
	b[ 6].se  = &b[10];
	b[ 6].sw  = &b[9];
	b[ 6].nw  = &b[1];

	b[ 7].occ = 1;
	b[ 7].ne  = &b[3];
	b[ 7].se  = &b[11];
	b[ 7].sw  = &b[10];
	b[ 7].nw  = &b[2];



	b[ 8].occ = 1;
	b[ 8].ne  = &b[5];
	b[ 8].se  = &b[13];
	b[ 8].sw  = &b[12];
	b[ 8].nw  = &b[4];

	b[ 9].occ = 1;
	b[ 9].ne  = &b[6];
	b[ 9].se  = &b[14];
	b[ 9].sw  = &b[13];
	b[ 9].nw  = &b[5];

	b[10].occ = 1;
	b[10].ne  = &b[7];
	b[10].se  = &b[15];
	b[10].sw  = &b[14];
	b[10].nw  = &b[6];

	b[11].occ = 1;
	b[11].ne  = NULL;
	b[11].se  = NULL;
	b[11].sw  = &b[15];
	b[11].nw  = &b[7];



	b[12].occ = 0;
	b[12].ne  = &b[8];
	b[12].se  = &b[16];
	b[12].sw  = NULL;
	b[12].nw  = NULL;

	b[13].occ = 0;
	b[13].ne  = &b[9];
	b[13].se  = &b[17];
	b[13].sw  = &b[16];
	b[13].nw  = &b[8];

	b[14].occ = 0;
	b[14].ne  = &b[10];
	b[14].se  = &b[18];
	b[14].sw  = &b[17];
	b[14].nw  = &b[9];

	b[15].occ = 0;
	b[15].ne  = &b[11];
	b[15].se  = &b[19];
	b[15].sw  = &b[18];
	b[15].nw  = &b[10];



	b[16].occ = 0;
	b[16].ne  = &b[13];
	b[16].se  = &b[21];
	b[16].sw  = &b[20];
	b[16].nw  = &b[12];

	b[17].occ = 0;
	b[17].ne  = &b[14];
	b[17].se  = &b[22];
	b[17].sw  = &b[21];
	b[17].nw  = &b[13];

	b[18].occ = 0;
	b[18].ne  = &b[15];
	b[18].se  = &b[23];
	b[18].sw  = &b[22];
	b[18].nw  = &b[14];

	b[19].occ = 0;
	b[19].ne  = NULL;
	b[19].se  = NULL;
	b[19].sw  = &b[23];
	b[19].nw  = &b[15];



	b[20].occ = -1;
	b[20].ne  = &b[16];
	b[20].se  = &b[24];
	b[20].sw  = NULL;
	b[20].nw  = NULL;

	b[21].occ = -1;
	b[21].ne  = &b[17];
	b[21].se  = &b[25];
	b[21].sw  = &b[24];
	b[21].nw  = &b[16];

	b[22].occ = -1;
	b[22].ne  = &b[18];
	b[22].se  = &b[26];
	b[22].sw  = &b[25];
	b[22].nw  = &b[17];

	b[23].occ = -1;
	b[23].ne  = &b[19];
	b[23].se  = &b[27];
	b[23].sw  = &b[26];
	b[23].nw  = &b[18];



	b[24].occ = -1;
	b[24].ne  = &b[21];
	b[24].se  = &b[29];
	b[24].sw  = &b[28];
	b[24].nw  = &b[20];

	b[25].occ = -1;
	b[25].ne  = &b[22];
	b[25].se  = &b[30];
	b[25].sw  = &b[29];
	b[25].nw  = &b[21];

	b[26].occ = -1;
	b[26].ne  = &b[23];
	b[26].se  = &b[31];
	b[26].sw  = &b[30];
	b[26].nw  = &b[22];

	b[27].occ = -1;
	b[27].ne  = NULL;
	b[27].se  = NULL;
	b[27].sw  = &b[31];
	b[27].nw  = &b[23];



	b[28].occ = -1;
	b[28].ne  = &b[24];
	b[28].se  = NULL;
	b[28].sw  = NULL;
	b[28].nw  = NULL;

	b[29].occ = -1;
	b[29].ne  = &b[25];
	b[29].se  = NULL;
	b[29].sw  = NULL;
	b[29].nw  = &b[24];

	b[30].occ = -1;
	b[30].ne  = &b[26];
	b[30].se  = NULL;
	b[30].sw  = NULL;
	b[30].nw  = &b[25];

	b[31].occ = -1;
	b[31].ne  = &b[27];
	b[31].se  = NULL;
	b[31].sw  = NULL;
	b[31].nw  = &b[26];

	int i;
	for (i = 0; i < 32; i++) b[i].sqNum = i;

	return;
}

// ===== AI ===== //

/*
// This was used to display the move code during debugging
void printr(uint32 move) {
	printf("%x ", (move / 0x4000) & 0x1F);
	printf("%x ", (move /  0x200) & 0x1F);
	printf("%x ", (move /   0x10) & 0x1F);
	printf("%x ", (move / 	  8)  &    1);
	printf("%x\n", (move)		  &    7);
	return;
}
*/

// Degenrate case calculations, no recursion
int getHeuristic(uint32 white, uint32 black, uint32 kings) {
	uint32 wk = kings & white;
	uint32 bk = kings & black;
	uint32 wm = white - wk;
	uint32 bm = black - bk;

	int i, heur = 0;

	for (i = 0; i < 32; i++) {
		heur += 2 * ((int)(bk % 2) - (int)(wk % 2)) + ((int)(bm % 2) - (int)(wm % 2));
		wk /= 2;
		bk /= 2;
		wm /= 2;
		bm /= 2;
	}
	return heur;
}

// Does the recursion that getHeuristic() is missing
int hpopulate(Node * node) {
	if (!node) return 0;

	Node * ptr = node;
	if (!ptr->chl) {
		ptr->heur = getHeuristic(ptr->white, ptr->black, ptr->kings);
		return ptr->heur;
	}

	int min =  25;
	int max = -25;
	int tmp;

	min = max = hpopulate(ptr->chl);

	while (ptr) {
		tmp = hpopulate(ptr->sib);
		if (tmp > max) max = tmp;
		if (tmp < min) min = tmp;
		ptr = ptr->sib;
	}

	node->heur = (node->chl->info & 8) ? max : min;
	return node->heur;

}

// array of pieces that can move
uint32 canMove(int origin, uint32 white, uint32 black, uint32 kings) {
	uint32 target = 0;
	uint32 bitsrc = pwr(2, origin);
	//printf("....%d..", origin);
	if (bitsrc & (white | kings)) {
		target += (ne[origin] & (~(black | white)));
		target += (nw[origin] & (~(black | white)));
	}
	if (bitsrc & (black | kings)) {
		target += (se[origin] & (~(black | white)));
		target += (sw[origin] & (~(black | white)));
	}
	//printf("..%x\n", target);
	return target;
}

// array of pieces that can jump
uint32 canJump(int origin, uint32 white, uint32 black, uint32 kings) {
	uint32 target = 0;
	uint32 bitsrc = pwr(2, origin);
	//printf("....%d..", origin);
	if (bitsrc & (white &  kings)) { // need to check square being jumped
		if (origin > 7  && origin % 4 != 3) {
			if (ne[origin] & black)
				target += (pwr(2, origin - 7) & (~(black | white)));
		}
		if (origin < 24 && origin % 4 != 3) {
			if (se[origin] & black)
				target += (pwr(2, origin + 9) & (~(black | white)));
		}
		if (origin < 24 && origin % 4 != 0) {
			if (sw[origin] & black)
				target += (pwr(2, origin + 7) & (~(black | white)));
		}
		if (origin > 7  && origin % 4 != 0) {
			if (nw[origin] & black)
				target += (pwr(2, origin - 9) & (~(black | white)));
		}
	}
	if (bitsrc & (black &  kings)) {
		if (origin > 7  && origin % 4 != 3) {
			if (ne[origin] & white)
				target += (pwr(2, origin - 7) & (~(black | white)));
		}
		if (origin < 24 && origin % 4 != 3) {
			if (se[origin] & white)
				target += (pwr(2, origin + 9) & (~(black | white)));
		}
		if (origin < 24 && origin % 4 != 0) {
			if (sw[origin] & white)
				target += (pwr(2, origin + 7) & (~(black | white)));
		}
		if (origin > 7  && origin % 4 != 0) {
			if (nw[origin] & white)
				target += (pwr(2, origin - 9) & (~(black | white)));
		}
	}
	if (bitsrc & (white & ~kings)) {
		if (origin > 7  && origin % 4 != 3) {
			if (ne[origin] & black)
				target += (pwr(2, origin - 7) & (~(black | white)));
		}
		if (origin > 7  && origin % 4 != 0) {
			if (nw[origin] & black)
				target += (pwr(2, origin - 9) & (~(black | white)));
		}
	}
	if (bitsrc & (black & ~kings)) {
		if (origin < 24 && origin % 4 != 3) {
			if (se[origin] & white)
				target += (pwr(2, origin + 9) & (~(black | white)));
		}
		if (origin < 24 && origin % 4 != 0) {
			if (sw[origin] & white)
				target += (pwr(2, origin + 7) & (~(black | white)));
		}
	}
	//printf("..%x\n", target);
	return target;
}

uint32 jumpedSquare(int src, int dst) {

	if (src - dst ==  7) return lg(ne[src]);
	if (src - dst == -9) return lg(se[src]);
	if (src - dst == -7) return lg(sw[src]);
	if (src - dst ==  9) return lg(nw[src]);
	return 0;
}

// Gets one move at a time.  When the return value from a previous call
	// is passed as the "left" parameter, it finds the next move.
	// Moves are ordered by source and then destination.
uint32 getMoves(uint32 previous, uint32 left, uint32 white, uint32 black, uint32 kings) {

	uint32 turn, crown = 0;
	uint32 level = (previous & 7) - 1;
	int src = (int)((left / 0x200) & 0x1F);
	int dst = (int)((left / 0x010) & 0x1F);
	int sel = src + 1;

	if (previous & 0x7C000 && !(previous & 0x80000) &&
		canJump(dst, white, black, kings)) {
			// double jumps

		turn = 8 &  previous;
		uint32 jumps = canJump(dst, white, black, kings);
		int d = 0;
		while(jumps && jumps % 2 == 0) {
			d++;
			jumps /= 2;
		}
		if (((turn) ? (d > 27) : (d < 4)) && !(kings & pwr(2, d)))
			crown = 0x80000;
		return crown + dst * 0x200 + d * 0x10 + turn + level +
				0x4000 * jumpedSquare(dst, d);

	}
	else {
		turn = 8 & ~previous;
	}

	if (!left || left & 0x7C000) {
			// Check for jumps first, since jumping is mandatory
		if (canJump(src, white, black, kings) &&
			(pwr(2, src) & ((turn) ? black : white)) &&
			canJump(src, white, black, kings) - pwr(2, dst) > pwr(2, dst)) {
				// more jumps from this square

			uint32 jumps = canJump(src, white, black, kings);
			int d = 0;
			while((jumps && jumps % 2 == 0 && d > dst) || (left && d <= dst)) {
				d++;
				jumps /= 2;
			}
			if (((turn) ? (d > 27) : (d < 4)) && !(kings & pwr(2, d)))
				crown = 0x80000; // if a new king is crowned
			return crown + src * 0x200 + d * 0x10 + turn + level +
					0x4000 * jumpedSquare(src, d);
		}
		while (sel < 32) {
			if ((pwr(2, sel) & ((turn) ? black : white)) &&
				canJump(sel, white, black, kings)) {

				uint32 jumps = canJump(sel, white, black, kings);
				int d = 0;
				if (jumps) {
					while(!(jumps % 2)) {
						d++;
						jumps /= 2;
					}
				}
				if (((turn) ? (d > 27) : (d < 4)) && !(kings & pwr(2, d)))
					crown = 0x80000;
				return crown + sel * 0x200 + d * 0x10 + turn + level +
					0x4000 * jumpedSquare(sel, d);
			}
			sel++;
		}
	}

	if (left & 0x7C000) return 0; // If a previous available move is a jump,
							// we must jump, a simple move would be illegal

	// "else", but not really, because if !left and no jumps need to move
		// can't jump so check moves
	sel = src + 1;
	if (canMove(src, white, black, kings) &&
		(pwr(2, src) & ((turn) ? black : white)) &&
		canMove(src, white, black, kings) - pwr(2, dst) > pwr(2, dst)) {
		// more moves from this square
		uint32 moves = canMove(src, white, black, kings);
		int d = 0;
		while((moves && moves % 2 == 0 && d > dst) || (left && d <= dst)) {
			d++;
			moves /= 2;
		}
		if (((turn) ? (d > 27) : (d < 4)) && !(kings & pwr(2, d)))
			crown = 0x80000;
		return crown + src * 0x200 + d * 0x10 + turn + level;
	}

	while (sel < 32) {
		if ((pwr(2, sel) & ((turn) ? black : white)) &&
			canMove(sel, white, black, kings)) {

			uint32 moves = canMove(sel, white, black, kings);
			int d = 0;
			if (moves) {
				while(!(moves % 2)) {
					d++;
					moves /= 2;
				}
			}
			//printf("%d - ", sel);
			if (((turn) ? (d > 27) : (d < 4)) && !(kings & pwr(2, d)))
				crown = 0x80000;
			return crown + sel * 0x200 + d * 0x10 + turn + level;
		}
		sel++;
	}
	//printf("no moves\n");
	// already gave all available moves
	return 0;
}

// creates a singular node, used in children()
Node * build(uint32 move, uint32 white, uint32 black, uint32 kings) {

	if (move < 16) return NULL;

	Node * n = (Node *)malloc(sizeof(Node));
	uint32 w = white;
	uint32 b = black;
	uint32 k = kings;

	if (move & 0x7C000) { // if jump
		uint32 jumped = move & 0x7C000 / 0x4000;
		jumped = ~pwr(2, (int)jumped); // notice the binary NOT
		w = w & jumped;
		b = b & jumped;
		k = k & jumped;
	}

	uint32 src = (move & 0x3E00) / 0x200;
	src = pwr(2, (int)src);
	uint32 dst = (move & 0x1F0) / 0x10;
	dst = pwr(2, (int)dst);

	// need to crown new kings
	if (move & 0x80000) k = k | dst;

	// actually moving the pieces
	if (move & 8) b = (b & ~src) | dst;
	else		  w = (w & ~src) | dst;
	if (k & src)  k = (k & ~src) | dst;

	n->white = w;
	n->black = b;
	n->kings = k;
	n->info  = move;
	n->heur  = -26;

	return n;
}

// Recursively creates all the children for a node, and their children,...
Node * children(uint32 former, uint32 white, uint32 black, uint32 kings) {// turn?
	Node * ptr = NULL;
	Node * tmp = NULL;
	uint32 move = getMoves(former, 0, white, black, kings);
	if (!move) return NULL;
	if (!(move & 7)) return NULL; // reached terminal depth of tree
	while (move) {
		tmp = build(move, white, black, kings);
		if (tmp->info & 7) {
			tmp->chl  = children(move, tmp->white, tmp->black, tmp->kings);
			hpopulate(tmp);
		}
		else {
			tmp->chl  = NULL;
			tmp->heur = getHeuristic(tmp->white, tmp->black, tmp->kings);
		}
		tmp->sib = ptr;
		ptr = tmp;

		// This is why getMoves() had to be so complex, so that we can
			// check every available move in a useful sequence
		move = getMoves(former, move, white, black, kings);
	}
	return ptr;
}

void destroy(Node * node) {
	if (node->chl) destroy(node->chl);
	if (node->sib) destroy(node->sib);
	free(node);
	return;
}

uint32 chooseMove(Node * tree) {
	Node * tmp = tree->chl;
	while (tmp->sib && tmp->heur != tree->heur) tmp = tmp->sib;

	uint32 info = tmp->info;
	destroy(tree);	// I know, rebuilding the tree each time is more work,
						// but it simplifies the heuristics calculations
						// and minimax algorithms
	return info;
}

// Updates tree to account for what the human player did
Node * humanMove(int src, int dst, int jmp, Square board[32]) {
	int i;
	uint32 white = 0;
	uint32 black = 0;
	uint32 kings = 0;
	uint32 info  = 0;
	for (i = 0; i < 32; i++) {
		if (board[i].occ < 0) white += pwr(2, i);
		if (board[i].occ > 0) black += pwr(2, i);
		if (board[i].occ == 2 || board[i].occ == -2)
			kings += pwr(2, i);
	}

	info = jmp * 0x4000 + src * 0x200 + dst * 0x10 + 8 + DIFF;
	Node * n = children(info, white, black, kings);
	return n;
}

// ===== Other =====

// exponentiation / binary decode
uint32 pwr(int base, int exp) {
	int i;
	uint32 r = 1;
	for (i = 0; i < exp; i++) r *= base;
	return r;
}

// logarithm / binary encode
int lg(uint32 k) {
	if (!k) return 0;
	int i = -1;
	while (k) {
		k /= 2;
		i++;
	}
	return i;
}
