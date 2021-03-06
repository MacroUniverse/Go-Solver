#pragma once
#include "tree.h"

Int Tree::solve(Long_I treeInd)
{
	// trivially solved
	if (solution(treeInd) != Sol::UNKNOWN) {
		if (solved(treeInd)) {
			// solved node
			return 0;
		}
		else if (solution(treeInd) == Sol::FORBIDDEN) {
			// forbidden node
			return 3;
		}
	}

	Bool debug_stop = nnode() >= 2000; // debug
	static Bool save = false; // debug

	static Bool auto_solve = false;
	Int i, solve_ret;
	MovRet move_ret;
	
	Long child_treeInd;
	Int child_sco2;
	Sol child_sol;

	// ko related
	Bool has_ko_link = false, has_ko_child = false;
	Int best_solvable_child_sco2 = -1;
	Sol best_solvable_child_sol = Sol::BAD;
	Int best_ko_child_sco2 = -1;
	static Long auto_solve_treeInd = 1000000;

	// set mark
	++m_nodes[treeInd].mark();

	// enumerate children
	for (i = 0; i < 100000; ++i) {
		// make a move
		debug_stop = nnode() >= 2000;
		if (i < m_nodes[treeInd].nnext()) {
			// check existing child
			child_treeInd = m_nodes[treeInd].next(i)->to();
			if (m_nodes[treeInd].next(i)->isko()) {
				// existing ko link, check if linked to path
				Linkp merge_link;
				Int ret = islinked(merge_link, m_nodes[treeInd].next(i)->to(), treeInd);
				if (ret == 2) { // on path ko link
					move_ret = MovRet::ON_PA_KO_LN;
				}
				else if (ret == 3) { // off path ko link, shift it
					m_nodes[treeInd].next(i)->ko_link_2_link();
					merge_link->link_2_ko_link();
					move_ret = MovRet::OFF_PA_KO_LN;
				}
				else {
					error("unknown!");
				}
			}
			else {
				// existing child
				move_ret = MovRet::NEW_ND;
			}
		}
		else {
			// look for a new child (all existing children checked)
			// prompt_move() or rand_smart_move
			if (treeInd >= auto_solve_treeInd) {
				move_ret = rand_smart_move(child_treeInd, treeInd);
			}
			else {
				if (auto_solve) {
					// auto solve successful!
					cout << "node " << auto_solve_treeInd << " solution : " << solution(auto_solve_treeInd) << "\n" << endl;
					writeSGF("test.sgf"); // debug
					auto_solve_treeInd = 1000000;
				}
				cout << "current tree index : " << treeInd << endl;
				disp_board(treeInd);
				auto_solve = inp.Bool("auto solve this node?");

				if (auto_solve) {
					auto_solve_treeInd = treeInd;
					move_ret = rand_smart_move(child_treeInd, treeInd);
				}
				else
					move_ret = prompt_move(child_treeInd, treeInd);
			}
		}

		// made a move
		if (save) { // debug
			writeSGF("test.sgf");
			save = false;
		} // end debug

		if (move_ret == MovRet::NEW_ND || move_ret == MovRet::LINK
			|| move_ret == MovRet::OFF_PA_KO_LN) {

			Linkp plink = next(treeInd, -1);

			// debug, display board
			if (move_ret == MovRet::NEW_ND) {
				cout << "largest treeInd = " << max_treeInd() << endl;
				disp_board(max_treeInd()); cout << "\n\n" << endl;
				cout << ""; // debug break point
			}
			// end debug

			solve_ret = -1000;
			if (move_ret == MovRet::LINK && is_ko_node(child_treeInd)) {
				// linked to a ko node
				if (check_clean_ko_node(child_treeInd) >= 0) {
					// linked to a clean ko node
					solve_ret = 2;
				}
				else {
					// linked to an unclean ko node
					// shift ko links when necessary
					Int a = 0; // debug break point
				}
			}
			
			if (solve_ret == -1000)
				//###########################################################
				solve_ret = solve(child_treeInd);
				//###########################################################

			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug

			if (solve_ret == -1) {
				// is an unclean ko child
				if (solution(child_treeInd) == Sol::KO_GOOD) {
					// is a good ko child
					set_score2(inv_score2(score2(child_treeInd)), treeInd);
					set_solution(Sol::KO_BAD, treeInd);
					resolve_ko(treeInd);
					if (is_new_clean_ko_node(treeInd)) {
						// is clean ko node
						push_clean_ko_node(treeInd);
						--m_nodes[treeInd].mark();
						return 2;
					}
					else {
						// not clean ko node
						--m_nodes[treeInd].mark();
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
				--m_nodes[treeInd].mark();
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
						set_solution(Sol::BAD, treeInd);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd);
						resolve_ko(treeInd);
						--m_nodes[treeInd].mark();
						return 0;
					}
				}
			}
			else
				error("unhandled case!");
		}
		else if (move_ret == MovRet::ON_PA_KO_LN || move_ret == MovRet::DB_PAS_KO_LN) {
			Long child_treeInd = next(treeInd, -1)->to();
			has_ko_link = true;
			continue;
		}
		else if (move_ret == MovRet::ALL_EXIST) {
			// no good child or good ko child is found
			if (!has_ko_link && !has_ko_child) {
				if (best_solvable_child_sco2 < 0) {
					// all children are forbidden
					set_solution(Sol::FORBIDDEN, treeInd);
					resolve_ko(treeInd);
					--m_nodes[treeInd].mark();
					return 3;
				}
				// all children solved
				if (best_solvable_child_sol == Sol::FAIR) {
					set_solution(Sol::FAIR, treeInd);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd);
				}
				else { // best == Sol::BAD
					set_solution(Sol::GOOD, treeInd);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd);
				}
				resolve_ko(treeInd);
				--m_nodes[treeInd].mark();
				return 0; // debug break point
			}
			// not all children solvable
			else if (has_ko_link && !has_ko_child) {
				// has ko link, no ko child
				if (best_solvable_child_sco2 < 0) {
					// forbidden node
					set_solution(Sol::FORBIDDEN, treeInd);
					resolve_ko(treeInd);
					--m_nodes[treeInd].mark();
					return 3;
				}
				set_score2(inv_score2(best_solvable_child_sco2), treeInd);
				calc_ko_sol(treeInd);
				--m_nodes[treeInd].mark();
				return -1;
			}
			else if (!has_ko_link && has_ko_child) {
				// has ko child, no ko link
				if (best_solvable_child_sco2 >= best_ko_child_sco2) {
					// trivial ko
					set_score2(inv_score2(best_solvable_child_sco2), treeInd);
					calc_sol(treeInd);
					resolve_ko(treeInd);
					--m_nodes[treeInd].mark();
					return 0;
				}
				else {
					// this is a ko node
					set_score2(inv_score2(best_ko_child_sco2), treeInd);
					calc_ko_sol(treeInd);
					resolve_ko(treeInd);
					if (is_new_clean_ko_node(treeInd)) {
						// is clean ko node
						push_clean_ko_node(treeInd);
						--m_nodes[treeInd].mark();
						return 2;
					}
					else {
						--m_nodes[treeInd].mark();
						return -1;
					}
				}
			}
			else { // (has_ko_link && has_ko_child)
				   // has both ko child and ko link
				set_score2(inv_score2(MAX(best_solvable_child_sco2, best_ko_child_sco2)), treeInd);
				calc_ko_sol(treeInd);
				resolve_ko(treeInd);
				--m_nodes[treeInd].mark();
				return -1;
			}
		}
		else if (move_ret == MovRet::DB_PAS_END) {
			--m_nodes[treeInd].mark();
			return 1;
		}
		else
			error("unhandled return!");
	}
	error("unkown error!");
	return -1;
}
