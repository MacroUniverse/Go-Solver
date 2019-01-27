// solve an existing ko node or make it a clean ko node
// try to solve a ko node (treeInd_to) when it has been linked
// the new link is from treeInd_from
// return 0 if solved to solvable node
// return 1 if solved to a clean ko node and pushed
// return 2 if this is already a clean ko node
Int solve_ko(Long_I treeInd_from, Int_I forkInd);

// internal recursive function called by solve_ko()
// solve a ko node
// this function is similar to solve()
// return 0 if solved
// return 1 if is a non-clean ko node
Int solve_ko0(Long_I treeInd1);


inline Int Tree::solve_ko(Long_I treeInd_from, Int_I forkInd)
{
	Int i, ret;
	Long treeInd_to = next(treeInd_from, forkInd);

	

	// Now we can start solving!
	ret = solve_ko0(treeInd_to);
	return ret;
}

Int Tree::solve_ko0(Long_I treeInd1)
{
	Int i, solve_ret, move_ret;
	Int child_treeInd, child_sco2, Nchild = node(treeInd1).nnext();
	Int best_solvable_child_sco2 = -1, best_ko_child_sco2 = -1;
	Sol child_sol;
	Sol best_solvable_child_sol = Sol::BAD, best_ko_child_sol = Sol::BAD;
	Bool has_ko_link = false, has_ko_child = false, save = false;

	// check existing children
	for (i = 0; i < Nchild; ++i) {
		child_treeInd = node(treeInd1).next(i);
		if (is_ko_child(treeInd1, i)) {
			// found a ko child
			has_ko_child = true;
			solve_ret = solve_ko0(child_treeInd);
			if (solve_ret == 0) {
				// ko child is solved
				child_sco2 = score2(child_treeInd);
				child_sol = solution(child_treeInd);
				if (best_solvable_child_sco2 < child_sco2) {
					best_solvable_child_sco2 = child_sco2;
					best_solvable_child_sol = child_sol;
					if (child_sol == Sol::GOOD) {
						set_solution(Sol::BAD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						rm_clean_ko_node(treeInd1);
						resolve_ko(treeInd1);
						return 0;
					}
				}
				continue;
			}
			else if (solve_ret == 1) {
				// is a non-clean ko child
				if (solution(child_treeInd) == Sol::KO_GOOD) {
					// is a good ko child
					set_score2(inv_score2(score2(child_treeInd)), treeInd1);
					set_solution(Sol::KO_BAD, treeInd1);
					resolve_ko(treeInd1);
					if (is_new_clean_ko_node(treeInd1)) {
						error("this should not happen!");
					}
					else {
						// not clean ko node
						return 1;
					}
				}
				else {
					child_sco2 = score2(child_treeInd);
					if (best_ko_child_sco2 < child_sco2) {
						best_ko_child_sco2 = child_sco2;
					}
					has_ko_child = true;
					continue;
				}
			}
			else {
				error("unhandled case");
			}
		}
		else if (node(treeInd1).is_next_ko_link(i)) {
			// found a ko link
			m_ko_link_from.push_back(treeInd1);
			m_ko_link_to.push_back(child_treeInd);
			has_ko_link = true;
			continue;
		}
		else {
			// found a solvable child (solved)
			child_sco2 = score2(child_treeInd);
			child_sol = solution(child_treeInd);
			if (best_solvable_child_sco2 < child_sco2) {
				best_solvable_child_sco2 = child_sco2;
				best_solvable_child_sol = child_sol;
				if (child_sol == Sol::GOOD) {
					set_solution(Sol::BAD, treeInd1);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
					rm_clean_ko_node(treeInd1);
					return 0;
				}
			}
		}
	}

	if (solution(treeInd1) == Sol::KO_BAD) {
		for (i = 0; i < 1000; ++i) {
			// not all children checked
			// try to create other children
			move_ret = rand_smart_move(treeInd1);
			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug
			if (move_ret == 0 || move_ret == 1) {
				// 0: successful and new node created
				// 1: linked to existing node and no ko
				// begin debug
				cout << "largest treeInd = " << max_treeInd() << endl;
				disp_board(max_treeInd()); cout << "\n\n" << endl;
				cout << ""; // debug break point
				// end debug
				child_treeInd = next(treeInd1, -1);

				//#################################
				solve_ret = solve(child_treeInd);
				//#################################

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
							return 0; // debug break point
						}
					}
				}
				else
					error("unhandled case!");
			}
			else if (move_ret == 2 || move_ret == -3) {
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
			else if (move_ret == -2) {
				// passed and game ends
				error("unhandled case!");
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
					rm_clean_ko_node(treeInd1);
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
						rm_clean_ko_node(treeInd1);
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
		}
	}

	// all children are checked
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
		rm_clean_ko_node(treeInd1);
		return 0; // debug break point
	}
	// not all children solvable
	if (has_ko_link && !has_ko_child) {
		// has ko link, no ko child
		if (best_solvable_child_sco2 < 0) {
			error("impossible case!");
		}
		set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
		calc_ko_sol(treeInd1);
		return 1;
	}
	else if (!has_ko_link && has_ko_child) {
		// has ko child, no ko link
		if (best_solvable_child_sco2 >= best_ko_child_sco2) {
			// trivial ko
			set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
			calc_sol(treeInd1);
			resolve_ko(treeInd1);
			rm_clean_ko_node(treeInd1);
			return 0;
		}
		else {
			// this is a still a ko node
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
		return 1;
	}
}
