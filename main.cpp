#include "tree.h"

// initialize class static members
MatChar Board::m_mark; // 0: unmarked, 1: marked, else: not used yet
vector<Move> Board::m_group; // a group of connected stones
Int Board::m_qi; // qi of group

int main()
{
	// computer_vs_computer_ui();
	// human_vs_computer_ui();
	// human_vs_human_ui();

	board_Nx(3); board_Ny(3); // set board size
	komi2(17); // set koomi
	Tree tree;

	// debug: edit board here
	tree.place(1, 1);
	// end edit board

	tree.solve();
}
