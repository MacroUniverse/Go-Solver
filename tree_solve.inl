#pragma once
#include "tree.h"

Int Tree::solve(Long_I treeInd /*optional*/)
{
	Bool debug_stop = nnode() >= 137; // debug
	static Bool save = false; // debug

	static Bool auto_solve = false;
	Int i, solve_ret;
	MovRet move_ret;
	const Long treeInd1 = def(treeInd);
	Long child_treeInd;
	Int child_sco2;
	Sol child_sol;

	// ko related
	Bool has_ko_link = false, has_ko_child = false;
	Int best_solvable_child_sco2 = -1;
	Sol best_solvable_child_sol = Sol::BAD;
	Int best_ko_child_sco2 = -1;
	static Long auto_solve_treeInd = 1000000;

	// trivially solved
	if (solution(treeInd1) != Sol::UNKNOWN) {
		if (solved(treeInd1)) {
			// solved node
			return 0;
		}
		else if (solution(treeInd1) == Sol::FORBIDDEN) {
			// forbidden node
			return 3;
		}
	}

	// enumerate children
	for (i = 0; i < 100000; ++i) {
		debug_stop = nnode() >= 137;
		if (i < node(treeInd1).nnext()) {
			// check existing child
			child_treeInd = node(treeInd1).next(i);
			if (is_ko_node(child_treeInd)) {
				// existing ko child
				move_ret = MovRet::NEW_ND;
			}
			else if (node(treeInd1).is_next_ko_link(i)) {
				// existing ko link
				move_ret = MovRet::KO_LN;
			}
			else {
				// existing non-ko (solved/unknown/forbidden) child
				move_ret = MovRet::LN_NKO_ND;
			}
		}
		else {
			// look for a new child (all existing children checked)
			// prompt_move() or rand_smart_move
			child_treeInd = -1;
			if (treeInd1 >= auto_solve_treeInd) {
				move_ret = rand_smart_move(treeInd1);
			}
			else {
				if (auto_solve) {
					// auto solve successful!
					cout << "node " << auto_solve_treeInd << " solution : " << solution(auto_solve_treeInd) << "\n" << endl;
					writeSGF("test.sgf"); // debug
					auto_solve_treeInd = 1000000;
				}
				cout << "current tree index : " << treeInd1 << endl;
				disp_board(treeInd1);
				auto_solve = inp.Bool("auto solve this node?");

				if (auto_solve) {
					auto_solve_treeInd = treeInd1;
					move_ret = rand_smart_move(treeInd1);
				}
				else
					move_ret = prompt_move(treeInd1);
			}
		}

		if (save) { // debug
			writeSGF("test.sgf");
			save = false;
		} // end debug

		if (move_ret == MovRet::NEW_ND || move_ret == MovRet::LN_NKO_ND
			|| move_ret == MovRet::LN_CLN_KO_ND || move_ret == MovRet::LN_UCLN_KO_ND) {

			if (child_treeInd < 0) {
				child_treeInd = next(treeInd1, -1);
			}

			// debug, display board
			if (move_ret == MovRet::NEW_ND) {
				cout << "largest treeInd = " << max_treeInd() << endl;
				disp_board(max_treeInd()); cout << "\n\n" << endl;
				cout << ""; // debug break point
			}
			// end debug

			if (move_ret == MovRet::LN_UCLN_KO_ND) {
				// shift ko links to curren node
				shift_ko_links(treeInd1, -1);
			}
			
			if (move_ret == MovRet::LN_CLN_KO_ND) {
				// clean ko child
				solve_ret = 2;
			}
			else {
				//###########################################################
				solve_ret = solve(child_treeInd);
				//###########################################################
			}

			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug

			if (solve_ret == -1) {
				// is an unclean ko child
				if (solution(child_treeInd) == Sol::KO_GOOD) {
					// is a good ko child
					set_score2(inv_score2(score2(child_treeInd)), treeInd1);
					set_solution(Sol::KO_BAD, treeInd1);
					resolve_ko(treeInd1);
					if (is_new_clean_ko_node(treeInd1)) {
						// is clean ko node
						push_clean_ko_node(treeInd1);
						return 2;
					}
					else {
						// not clean ko node
						return -1;
					}
				}
				child_sco2 = score2(child_treeInd);
				if (best_ko_child_sco2 < child_sco2) {
					best_ko_child_sco2 = child_sco2;
				}
				has_ko_child = true;
				continue;
			}
			else if (solve_ret == 1) {
				// double passing solved two nodes
				return 0;
			}
			else if (solve_ret == 3) {
				// tried to solve forbidden node
				continue;
			}
			else if (solve_ret == 0 || solve_ret == 2) {
				if (solve_ret == 0) {
					// successful!
					child_sco2 = score2(child_treeInd);
					child_sol = solution(child_treeInd);
				}
				else { // (solve_ret == 2)
					// a clean ko node: treat as a normal node
					Int clean_ko_ind = check_clean_ko_node(child_treeInd);
					child_sco2 = clean_ko_node_sco2(clean_ko_ind);
					child_sol = clean_ko_node_sol(clean_ko_ind);
				}
				if (best_solvable_child_sco2 < child_sco2) {
					best_solvable_child_sco2 = child_sco2;
					best_solvable_child_sol = child_sol;
					if (best_solvable_child_sol == Sol::GOOD) {
						set_solution(Sol::BAD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						resolve_ko(treeInd1);
						return 0;
					}
				}
			}
			else
				error("unhandled case!");
		}
		else if (move_ret == MovRet::KO_LN || move_ret == MovRet::DB_PAS_KO_LN) {
			Long child_treeInd = next(treeInd1, -1);
			has_ko_link = true;
			if (solved(child_treeInd) || check_clean_ko_node(child_treeInd) >= 0) {
				// ko link target already solved
				resolve_ko1(treeInd1, child_treeInd);
			}
			continue;
		}
		else if (move_ret == MovRet::ALL_EXIST) {
			// no good child or good ko child is found
			if (!has_ko_link && !has_ko_child) {
				if (best_solvable_child_sco2 < 0) {
					// all children are forbidden
					set_solution(Sol::FORBIDDEN, treeInd1);
					resolve_ko(treeInd1);
					return 3;
				}
				// all children solved
				if (best_solvable_child_sol == Sol::FAIR) {
					set_solution(Sol::FAIR, treeInd1);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
				}
				else { // best == Sol::BAD
					set_solution(Sol::GOOD, treeInd1);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
				}
				resolve_ko(treeInd1);
				return 0; // debug break point
			}
			// not all children solvable
			else if (has_ko_link && !has_ko_child) {
				// has ko link, no ko child
				if (best_solvable_child_sco2 < 0) {
					// forbidden node
					set_solution(Sol::FORBIDDEN, treeInd1);
					resolve_ko(treeInd1);
					return 3;
				}
				set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
				calc_ko_sol(treeInd1);
				return -1;
			}
			else if (!has_ko_link && has_ko_child) {
				// has ko child, no ko link
				if (best_solvable_child_sco2 >= best_ko_child_sco2) {
					// trivial ko
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
					calc_sol(treeInd1);
					resolve_ko(treeInd1);
					return 0;
				}
				else {
					// this is a ko node
					set_score2(inv_score2(best_ko_child_sco2), treeInd1);
					calc_ko_sol(treeInd1);
					resolve_ko(treeInd1);
					if (is_new_clean_ko_node(treeInd1)) {
						// is clean ko node
						push_clean_ko_node(treeInd1);
						return 2;
					}
					else {
						return -1;
					}
				}
			}
			else { // (has_ko_link && has_ko_child)
				   // has both ko child and ko link
				set_score2(inv_score2(MAX(best_solvable_child_sco2, best_ko_child_sco2)), treeInd1);
				calc_ko_sol(treeInd1);
				resolve_ko(treeInd1);
				return -1;
			}
		}
		else if (move_ret == MovRet::DB_PAS_END) {
			return 1;
		}
		else
			error("unhandled return!");
	}
	error("unkown error!"); return -1;
}