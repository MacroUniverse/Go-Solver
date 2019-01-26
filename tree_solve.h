#pragma once
#include "tree.h"

Int Tree::solve(Long_I treeInd /*optional*/)
{
	Bool debug_stop = nnode() >= 2000; // nnode() >= 2492 || treeInd1 == 1; // debug
	Bool save = false;
	static Bool auto_solve = false;
	Int i, move_ret, solve_ret, solve_ko_ret, child_treeInd;
	const Long treeInd1 = def(treeInd);
	Int child_sco2;
	Sol child_sol;

	// ko related
	Bool has_ko_link = false, has_ko_child = false;
	Int best_solvable_child_sco2 = -1;
	Sol best_solvable_child_sol = Sol::BAD;
	Int best_ko_child_sco2 = -1;
	static Long auto_solve_treeInd = 1000000;

	// if already checked
	if (solution(treeInd1) != Sol::UNKNOWN) {
		if (is_ko_node(treeInd1)) {
			if (check_clean_ko_node(treeInd1)) {
				return 2;
			}
			else
				error("unclean ko node should be solved by solve_ko() not solve()!");
		}
		else if (solution(treeInd1) == Sol::FORBIDDEN) {
			return 3;
		}
		return 0;
	}

	// enumerate children
	for (i = 0; i < 100000; ++i) {
		debug_stop = nnode() >= 2000;

		// prompt_move() or rand_smart_move
		if (treeInd1 >= auto_solve_treeInd) {
			move_ret = rand_smart_move(treeInd1);
		}
		else {
			if (auto_solve) {
				// auto solve successful!
				cout << "node " << auto_solve_treeInd << " solution : " << solution(auto_solve_treeInd) << "\n" << endl;
				writeSGF("test.sgf");
				cout << "current tree ind : " << treeInd1 << endl;
				disp_board(treeInd1);
				auto_solve_treeInd = 1000000;
			}
			cout << "current tree index : " << treeInd1 << endl;
			disp_board(treeInd1);

			Char y_n;

			auto_solve = inp.Bool("auto solve this node?");

			if (auto_solve) {
				auto_solve_treeInd = treeInd1;
				move_ret = rand_smart_move(treeInd1);
			}
			else
				move_ret = prompt_move(treeInd1);
		}

		if (save) { // debug
			writeSGF("test.sgf");
			save = false;
		} // end debug

		if (move_ret == 0 || move_ret == 1 || move_ret == 3) {
			// 0: new node created
			// 1: linked to existing node
			child_treeInd = next(treeInd1, -1);
			// debug, display board
			if (move_ret == 0 || move_ret == 3) {
				cout << "largest treeInd = " << max_treeInd() << endl;
				disp_board(max_treeInd()); cout << "\n\n" << endl;
				cout << ""; // debug break point
			}
			// end debug

			//###########################################################
			solve_ret = solve(child_treeInd);
			//###########################################################

			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug

			if (solve_ret == -1) {
				// is a ko child
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
		else if (move_ret == 4) {
			// linked to a non-clean ko node
			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug
			  // break the link first
			child_treeInd = next(treeInd1, -1);

			//#####################################
			solve_ko_ret = solve_ko(treeInd1, -1);
			//#####################################

			if (solve_ko_ret == 0 || solve_ko_ret == 1) {
				// 0: solved to solvable node
				// 1: solved to clean ko node and pushed
				if (solve_ko_ret == 0) {
					child_sco2 = score2(child_treeInd);
					child_sol = solution(child_treeInd);
				}
				else {
					child_sco2 = clean_ko_node_sco2(child_treeInd);
					child_sol = clean_ko_node_sol(child_treeInd);
				}
				if (best_solvable_child_sco2 < child_sco2) {
					best_solvable_child_sco2 = child_sco2;
					best_solvable_child_sol = child_sol;
					if (best_solvable_child_sol == Sol::GOOD) {
						set_solution(Sol::BAD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						resolve_ko(treeInd1);
						return 0; // debug break point
					}
				}
				continue;
			}
			else if (solve_ko_ret == 2) {
				error("clean ko node should have catched by ");
			}
			else {
				error("unhandled case!");
			}
		}
		else if (move_ret == 2 || move_ret == -3) {
			// a ko link created
			// 2: ko link created
			// -3: double pass not allowed and ko link created
			Long child_treeInd = next(treeInd1, -1);
			has_ko_link = true;
			if (solved(child_treeInd) || check_clean_ko_node(child_treeInd) >= 0) {
				// ko link target already solved
				resolve_ko1(treeInd1, child_treeInd);
			}
			continue;
		}
		else if (move_ret == -1) {
			// all children exist
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
		else if (move_ret == -2) {
			// double passing caused game end.
			return 1;
		}
		else
			error("unhandled return!");
	}
	error("unkown error!"); return -1;
}
