//#include "ui.h"
#include "tree.h"

int main()
{
	// computer_vs_computer_ui();
	// human_vs_computer_ui();
	// human_vs_human_ui();

	board_Nx(4); board_Ny(4); // set board size
	komi2(31); // set koomi
	Tree tree;

	// debug: edit board here
	// tree.place(1, 1);
	// end edit board

	tree.solve(0);
	tree.writeSGF("test.sgf");
}
