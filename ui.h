#pragma once
#include "tree.h"

// run this for stand along program
void computer_vs_computer_ui()
{
	Int Nx, Ny;
	Doub komi;
	cout << "random game..." << endl;
	cout << "input board size x : "; cin >> Nx;
	cout << "input board size y : "; cin >> Ny;
	cout << "input komi : "; cin >> komi;
	board_Nx(Nx); board_Ny(Ny); // set board size
	komi2(round(komi * 2)); // set koomi
	Tree tree;

	tree.rand_game(0, true);
	getchar();
	getchar();
}

// run this for stand alone human vs computer
void human_vs_computer_ui()
{
	Int Nx, Ny, ret, i = 0, bscore4, x, y;
	Doub komi;
	Char color;
	cout << "human vs computer..." << endl;
	cout << "input board size x : "; cin >> Nx;
	cout << "input board size y : "; cin >> Ny;
	cout << "input komi : "; cin >> komi;
	cout << "your color : "; cin >> color;

	board_Nx(Nx); board_Ny(Ny); // set board size
	komi2(round(komi * 2)); // set koomi
	Tree tree;

	i = 0;
	cout << "\n\nstep " << i << " "; ++i;
	tree.disp_board();

	if (color == 'b' || color == 'B') {
		for (; i < 10000; ++i) {
			// human move
			cout << "input x y (negative to pass): ";
			cin >> x >> y;
			if (x < 0 || y < 0) {
				if (tree.pass()) // pass !
					break; // double pass !
			}
			if (tree.place(x, y)) {
				error("illegal move!");
			}
			cout << "step " << i << " ";
			tree.disp_board();

			// computer move
			ret = tree.rand_smart_move();
			if (ret == -1)
				error("unkown error!");
			if (ret == -2) {
				cout << "double pass !\n\n\n";
				break;
			}
			cout << "step " << i << " ";
			tree.disp_board();
		}
	}
	else if (color == 'w' || color == 'W') {
		for (; i < 10000; ++i) {
			// computer move
			ret = tree.rand_smart_move();
			if (ret == -1)
				error("unkown error!");
			if (ret == -2) {
				cout << "double pass !\n\n\n";
				break;
			}
			cout << "step " << i << " ";
			tree.disp_board();

			// human move
			cout << "input x y (negative to pass): ";
			cin >> x >> y;
			if (x < 0 || y < 0) {
				if (tree.pass()) // pass !
					break; // double pass !
			}
			if (tree.place(x, y)) {
				error("illegal move!");
			}
			cout << "step " << i << " ";
			tree.disp_board();
		}
	}
	else
		error("illegal color! must be 'b' or 'w'!");

	cout << "game over!" << "\n\n";

	tree.solve_end();

	if (tree.winner() == Who::BLACK) {
		cout << "black wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else if (tree.winner() == Who::WHITE) {
		cout << "white wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else { // draw
		cout << "draw!\n\n";
	}
	tree.writeSGF("record.sgf");

	getchar();
	getchar();
}

// run this for stand alone human vs computer
void human_vs_human_ui()
{
	Int Nx, Ny, ret, i = 0, bscore4, x, y;
	Doub komi;
	Char color;
	cout << "human vs computer..." << endl;
	cout << "input board size x : "; cin >> Nx;
	cout << "input board size y : "; cin >> Ny;
	cout << "input komi : "; cin >> komi;

	board_Nx(Nx); board_Ny(Ny); // set board size
	komi2(round(komi * 2)); // set koomi
	Tree tree;

	i = 0;
	cout << "\n\nstep " << i << " "; ++i;
	tree.disp_board();

	for (; i < 10000; ++i) {
		// human move
		cout << "input x y (negative to pass): ";
		cin >> x >> y;
		if (x < 0 || y < 0) {
			if (tree.pass()) // pass !
				break; // double pass !
		}
		if (tree.place(x, y)) {
			error("illegal move!");
		}
		cout << "step " << i << " ";
		tree.disp_board();
	}

	cout << "game over!" << "\n\n";

	tree.solve_end();

	if (tree.winner() == Who::BLACK) {
		cout << "black wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else if (tree.winner() == Who::WHITE) {
		cout << "white wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else { // draw
		cout << "draw!\n\n";
	}
	tree.writeSGF("record.sgf");

	getchar();
	getchar();
}
