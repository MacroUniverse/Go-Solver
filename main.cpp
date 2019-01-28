//#include "ui.h"
#include "tree.h"

// static member definitions
vector<Link> Linkp::m_links;

int main()
{
	// computer_vs_computer_ui();
	// human_vs_computer_ui();
	// human_vs_human_ui();

	board_Nx(3); board_Ny(3); // set board size
	komi2(17); // set koomi
	Tree tree;

	// debug: edit board here
	// tree.place(1, 1);
	// end edit board

	tree.solve(0);
	tree.writeSGF("test.sgf");
}
