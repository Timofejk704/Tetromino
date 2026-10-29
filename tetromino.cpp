#include <iostream>
#include <cctype>
#include <vector>
#include <cstdlib>

#include <pdcurses.h>

// to delay
#include <thread>
#include <chrono>

// to random
#include <random>


// some structs
struct cell {
	char ch; 
	int color_id;
};

struct values {
	// window
	int w_lines = 25, w_columns = 80;

	// field
	char none_char = '.';
	int f_lines = 20, f_columns = 10; 		// field size
	std::vector<std::vector<cell>> field;
	std::vector<std::vector<cell>> field_double;

	// tetromino
	int tet_idx = 0, rotation_idx = 0, color_idx = 0;
	int tet_line = 0, tet_column = f_columns / 2;
	bool tet_fall = true, tet_created = false;

	// other
	bool running = true;
	char user_input;
	int score = 0;
	int fall_speed = 3, tact = 0; 
	bool pause = true, defeat = false;
	int rem_line = f_lines;

	void Reset() {
		// create game field
		field = std::vector<std::vector<cell>> (f_lines + 1, std::vector<cell>(f_columns + 1, {none_char, 1}));
		field_double = std::vector<std::vector<cell>> (f_lines + 1, std::vector<cell>(f_columns + 1, {none_char, 1}));

		// reset variables value

		// field
		none_char = '.';
		f_lines = 20; f_columns = 10; 		// field size

		// tetromino
		tet_idx = 0; rotation_idx = 0; color_idx = 0;
		tet_line = 0; tet_column = f_columns / 2;
		tet_fall = true, tet_created = false;

		// other
		score = 0;
		fall_speed = 3; tact = 0; 
		pause = true; defeat = false;
		rem_line = f_lines;
		
	}
};

struct tetromino {
	char tet_char = '#';
	std::vector<std::vector<std::vector<int>>> tet_shape = {
		{ // I
			{1, 1, 1, 1},
			{0, 0, 0, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		},
		{ // J
			{1, 0, 0, 0},
			{1, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		},
		{ // L
			{0, 0, 1, 0},
			{1, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		},
		{ // O
			{0, 1, 1, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		},
		{ // S
			{0, 1, 1, 0},
			{1, 1, 0, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		},
		{ // T
			{0, 1, 0, 0},
			{1, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		},
		{ // Z
			{1, 1, 0, 0},
			{0, 1, 1, 0},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		},
	};
};


// a func that give random integer
int RandInt(int min, int max) {
	// static guarantees that the number generator will only be used once
    static std::random_device rd;	// create seed
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    
    return dist(gen);
};


// game loop funcs
bool IsValidMove(tetromino &tet, int _tet_idx, int _rotation_idx, int _tet_line, int _tet_column, std::vector<std::vector<cell>> _field_double) {

	for (int line = 0; line < tet.tet_shape[_tet_idx].size(); line++) {
		for (int column = 0; column < tet.tet_shape[_tet_idx][0].size(); column++) {
			if (tet.tet_shape[_tet_idx][line][column] == 1) {
				int n_tet_line = _tet_line, n_tet_column = _tet_column;

				if (_rotation_idx == 0) { n_tet_line += line; n_tet_column += column - 1; }
				else if (_rotation_idx == 1) { n_tet_line -= column - 1; n_tet_column += line; }
				else if (_rotation_idx == 2) { n_tet_line -= line; n_tet_column -= column - 1; }
				else if (_rotation_idx == 3) { n_tet_line += column - 1; n_tet_column -= line; }

				if (tet.tet_shape[_tet_idx][line][column] == 1 &&
				((n_tet_line <= 0 || n_tet_line - 1 >= _field_double.size() - 1 || n_tet_column >= _field_double[0].size()) ||
				(_field_double[n_tet_line][n_tet_column].ch == tet.tet_char))) return false;

			}
		}
	} return true;
};

void CreateTetromino(tetromino &tet, int _tet_idx, int _rotation_idx, int _color_idx, int _tet_line, int _tet_column, std::vector<std::vector<cell>> &_field_) {

	for (int line = 0; line < tet.tet_shape[_tet_idx].size(); line++) {
		for (int column = 0; column < tet.tet_shape[_tet_idx][0].size(); column++) {
			if (tet.tet_shape[_tet_idx][line][column] == 1) {
				int n_tet_line = _tet_line, n_tet_column = _tet_column;

				if (_rotation_idx == 0) { n_tet_line += line; n_tet_column += column - 1; }
				else if (_rotation_idx == 1) { n_tet_line -= column - 1; n_tet_column += line; }
				else if (_rotation_idx == 2) { n_tet_line -= line; n_tet_column -= column - 1; }
				else if (_rotation_idx == 3) { n_tet_line += column - 1; n_tet_column -= line; }
			
				_field_[n_tet_line][n_tet_column].ch = tet.tet_char;
				_field_[n_tet_line][n_tet_column].color_id = _color_idx;
			}
		}
	}
};

void GameLoop(tetromino &tet, int _tact, int &_fall_speed, int &_score, int &_tet_idx, int &_rotation_idx, int &_color_idx, int &_tet_line, int &_tet_column, bool &_tet_fall, bool &_tet_created, char &_none_char, std::vector<std::vector<cell>> &_field, std::vector<std::vector<cell>> &_field_double) {

	if (_score >= 5000) _fall_speed = 0;
	else if (_score >= 2000) _fall_speed = 1;
	else if (_score >= 500) _fall_speed = 2;

	int destroyed_lines = 0;
	for (int line = _field_double.size() - 1; line > 0; line--) {
		bool line_is_full = true;	
		for (int column = 0; column < _field_double[0].size(); column++) {
			if (_field_double[line][column].ch == _none_char) {
				line_is_full = false; 
				break;
			}
		} if (line_is_full) {
			destroyed_lines++;
			for (int _line = line; _line > 0; _line--) {
				for (int column = 0; column < _field_double[0].size(); column++) {
					_field_double[_line][column] = _field_double[_line - 1][column];
				}
			}
		} for (int column = 0; column < _field_double[0].size(); column++) {
			_field_double[0][column].ch = _none_char;
			_field_double[0][column].color_id = 1;
		} 
	} _score += destroyed_lines * 100;

	_field = _field_double;

	if (_tet_fall == true && _tet_created == false) {
		_color_idx = RandInt(2, 7); _tet_idx = RandInt(0, tet.tet_shape.size() - 1);
		CreateTetromino(tet, _tet_idx, _rotation_idx, _color_idx, _tet_line, _tet_column, _field);
		_tet_fall = false; _tet_created = true;
	} else {
		if (IsValidMove(tet, _tet_idx, _rotation_idx, _tet_line + 1, _tet_column, _field_double)) {
			if (_tact == _fall_speed) {
				_tet_line++; CreateTetromino(tet, _tet_idx, _rotation_idx, _color_idx, _tet_line, _tet_column, _field);
			} else CreateTetromino(tet, _tet_idx, _rotation_idx, _color_idx, _tet_line, _tet_column, _field);
		} else {
			_tet_fall = true; _tet_created = false;
			CreateTetromino(tet, _tet_idx, _rotation_idx, _color_idx, _tet_line, _tet_column, _field_double);
			_rotation_idx = 0; _tet_line = 0; _tet_column = _field_double[0].size() / 2;
		}
	}
};


// control funcs
void _DropTetromino(tetromino tet, int _tet_idx, int _rotation_idx, int &_tet_line, int _tet_column, std::vector<std::vector<cell>> &_field_double) {
	for (int line = _tet_line; line < _field_double.size(); line++) {
		if (IsValidMove(tet, _tet_idx, _rotation_idx, line, _tet_column, _field_double)) _tet_line = line;
	}
};

void _RotateTetromino(tetromino tet, int _tet_idx, int &_rotation_idx, int _tet_line, int _tet_column, std::vector<std::vector<cell>> _field_double) {
	if (IsValidMove(tet, _tet_idx, _rotation_idx, _tet_line, _tet_column, _field_double) && (_rotation_idx >= 3 || _tet_idx == 3)) _rotation_idx = 0;
	else if (IsValidMove(tet, _tet_idx, _rotation_idx + 1, _tet_line, _tet_column, _field_double) && _rotation_idx < 3) _rotation_idx++;
};


// output funcs
void GameOutput (values &vls) {

	int _score = vls.score;
	std::vector<std::vector<cell>> _field = vls.field;
	std::vector<std::vector<cell>> _field_double = vls.field_double;


	mvprintw(0, 0, "Tetromino by tmfjk_704");
	mvprintw(1, 0, "Score: %d", _score);

	for (int line = 0; line < _field.size(); line++) {
		for (int column = 0; column < _field[0].size(); column++) {

			attron(COLOR_PAIR(_field[line][column].color_id));		// color on
			mvprintw(line + 3, column + 3, "%c", _field[line][column].ch);
			attroff(COLOR_PAIR(_field[line][column].color_id));		// color off

			_field[line][column].ch = _field_double[line][column].ch;
			_field[line][column].color_id = _field_double[line][column].color_id;


		}
	}

	// guide
	mvprintw(5, 20, "Q - pause");
	mvprintw(6, 20, "R - rotate");
	mvprintw(7, 20, "A - left");
	mvprintw(8, 20, "D - right");
	mvprintw(9, 20, "SPACE - drop");


	attrset(A_NORMAL); 
	refresh();
};

void PauseMenuOutput (values &vls) {

	int _lines = vls.w_lines;
	int _columns = vls.w_columns;

	std::string message_1 = "Please switch to the English keyboard layout!";
	std::string message_2 = "Press SPACE to start/continue or Q to quit.";

	mvprintw((_lines / 2) - 1, (_columns - message_1.size()) / 2, "%s", message_1.c_str());
	mvprintw((_lines / 2), (_columns - message_2.size()) / 2, "%s", message_2.c_str());

	refresh();
};	


int main() {

	values vls;
	vls.Reset();

	tetromino tet;


	initscr();				// initialize window

	resize_term(vls.w_lines, vls.w_columns);

	cbreak();				// disables line buffering
	noecho();				// disables characters echo
	start_color();			// initialize colors
	keypad(stdscr, true);	// includes handling of special keys
	timeout(0);				// waits for character input without delay
	curs_set(0);			// set non-blinking cursor


	// color palette
	//			 Foreground   Background
    init_pair(1, COLOR_WHITE, COLOR_BLACK);

    init_pair(2, COLOR_BLUE, COLOR_BLUE);
    init_pair(3, COLOR_GREEN, COLOR_GREEN);
    init_pair(4, COLOR_CYAN, COLOR_CYAN);
    init_pair(5, COLOR_RED, COLOR_RED);
    init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(7, COLOR_YELLOW, COLOR_YELLOW);

    init_pair(8, COLOR_BLACK, COLOR_WHITE);

    init_pair(9, COLOR_WHITE, COLOR_WHITE);	


    while (vls.running) {

    	erase();	// func erase() cooler then clear()
    	vls.user_input = getch();							// get character
    	vls.user_input = std::tolower(vls.user_input); 		// any character to lowercase character

    	// defeat check
    	if (vls.pause == false && vls.defeat == false) {
	    	for (int column = 0; column < vls.field_double[0].size(); column++) {
	    		if (vls.field_double[1][column].ch != vls.none_char) {
	    			vls.defeat = true;
	    			break;
	    		}
	    	}
    	}

    	// if defeat
       	if (vls.pause == false && vls.defeat) {
    		for (int column = 0; column < vls.field_double[0].size(); column++)	
    			vls.field[vls.rem_line][column].color_id = 9; 
    		GameOutput(vls);
    		if (vls.field[0][vls.f_columns].color_id == 9) vls.pause = true;
    		vls.rem_line--;
    	} if (vls.pause == true && vls.defeat == true) vls.Reset();

    	// keyboard control
    	switch ((unsigned char)vls.user_input) {

    		case ' ': {																															// continuue game or drop
    			if (vls.pause) { vls.pause = false; break; }
    			else { _DropTetromino(tet, vls.tet_idx, vls.rotation_idx, vls.tet_line, vls.tet_column, vls.field_double); break; }
    		}

    		case 'q': {																															// pause or quit
    			if (vls.pause) { vls.running = false; break; }
    			else { vls.pause = true; break; }
    		}

    		case 'r': { if (vls.pause == false) _RotateTetromino(tet, vls.tet_idx, vls.rotation_idx, vls.tet_line, vls.tet_column, vls.field_double); break; }							// rotate
    		case 'a': { if (vls.pause == false && IsValidMove(tet, vls.tet_idx, vls.rotation_idx, vls.tet_line, vls.tet_column - 1, vls.field_double)) vls.tet_column--; break; }		// left
    		case 'd': { if (vls.pause == false && IsValidMove(tet, vls.tet_idx, vls.rotation_idx, vls.tet_line, vls.tet_column + 1, vls.field_double)) vls.tet_column++; break; }		// right

    	};

    	// if pause = true
    	if (vls.pause) PauseMenuOutput(vls);
    	else if (vls.pause == false && vls.defeat == false) {
    		GameLoop(tet, vls.tact, vls.fall_speed, vls.score, vls.tet_idx, vls.rotation_idx, vls.color_idx, vls.tet_line, vls.tet_column, vls.tet_fall, vls.tet_created, vls.none_char, vls.field, vls.field_double); 
    		GameOutput(vls);
    	}


    	// tact
    	std::this_thread::sleep_for(std::chrono::milliseconds(100));
    	if (vls.tact == vls.fall_speed) vls.tact = 0;
    	else vls.tact++;
    	// if the tetrominoes seem to be falling too slowly, simply set the value to less than 100 milliseconds in std::thread

    }

    endwin();
	return 0;
};