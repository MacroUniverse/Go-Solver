# Go-Solver
Solving the game of Go by analyzing the game tree.

## Introduction
Game of Go (Chinese: weiqi 围棋) is a board game. According to game theory, with the fair komi (贴目, an integer), a draw strategy is guaranteed to exists, with an unfair komi, a winning strategy is guaranteed to exist for the advantageous side. The mind that knows these strategies is called God of Go (or god).

The fair komi is currently only known for very small boards by human, the largest board is 7 by 7, calculated by Go experts, but with no rigorous proof. The largest board proven by computer is 5 by 5.

The game tree of Go reveals all possibilities of a game. The tree is composed of connected nodes, each node represents a status of the board. The node at the top of the game tree represents an empty board, and a node at the bottom of the tree represents configuration of the board when the game ends. A node may connect to multiple lower nodes (child nodes), but it can only connect to one parent node (the node that leads to it). Sibling nodes are the nodes that share the same parents.

Let's assume that two gods play the game starting from a given node in the tree with a given komi. According to game theory, if this is fair, the game will end with a draw, if this is unfair, the advantageous side is guaranteed to win.

The current objective of this project is to decide, for a given node and given komi, whether it is fair, and which side is advantageous (i.e. solving that node). Apparently, the closer the node is to the bottom, the easier it can be solved.

## Equivalent Board Configurations
* A board configuration specifies the positions of two kinds of stones on the board.
* Rotating the board does not change the configuration (square board can rotate 3 times and rectangle board 1 time).
* Inversing color of all stones does not change the configuration.
* For the same configuration, the solution depends on the player that leads to this configuration.
* A board configuration corresponds to two situations.
* A situation is a configuration with a specified player.
* A situation only have one solution (despite of ko).
* each node must have a unique situation, otherwise they should be combined.
* two nodes linked by a pass must corresponds to the two situations of one configuration.

## Algorithm
The solution of a given node (good/bad/fair for the one who played this node) is the opposite of the solution of it's best (good better than fair better than bad) child played by opponent, if the best child is good (i.e. at least a good child exists) then this node is bad. Otherwise, if the best children is fair, then this node is also fair. If the best child is bad (i.e. all children are bad), this node is good.

Thus the most efficient algorithm will depend on how well another program can play Go (i.e. how well it can guess the best child, second best child etc.). For a given node, we should always solve the (guessed) best children first, then the (guessed) second best, etc, keeping track of the best child in the meanwhile to decide when to stop. This is more efficient than randomly choosing a child if the given node is bad, but is equally efficient if this node is good or fair (since we have to solve all children anyway). Since we don't know, we should always solve the (guessed) best child first.

## Solution Independent of Komi
With a slightly different implementation, it's possible to solve a node without knowning the komi apriori (to distinguish, we call this scoring). Instead of deciding the solution (good/bad/fair) of a node directly, we can give it a score: the final score of the current player without komi (if two players are both god, and always choose the best scored move). It doesn't matter how this score is calculated (Chinese rule, Japanese rule, etc.). To socre a node, we must find the best scored child, and the score of the node will the the complementary (opponent's score for the same board configuration) score of it's best child. Whenever a komi is chosen, the solution for any solved node can be immediately calculated by a simple addition/subtraction.

However, this will require much more calculation because to score a node, all it's children must be scored. A compromise between solving and scoring will be to set a reasonable komi first, solve the given node and score it from the solved children, since probably not all children are solved, the score might be over estimated. If another komi is needed later, we might only need to score a few additional children of each node instead of starting all-over again or scoring all children.

## Computer Player Rules
* A dumb move is a bad move in all situation, and should never be played by the computer. Currently dumb moves include dumb eye filling and dumb big eye filling.
* An eye is a single empty point surrounded by the same color of stones. 4 stones are needed for an eye not on boarder, 3 for edge, 2 for corner.
* A Dumb eye filling is placing a stone in a player's own eye when it will not be destroyed by the opponent immediately. If this is allowed when enumerating child nodes, the computer will frequently commit suicide.
* A big eye is two connected empty points, surrounded by connected stones. It's not possible to destroy a big eye in one step.
* A Dumb big eye filling is placing a stone in a player's own big eye. At the end of the game, there will be no big eye left because the opponent will eventually try to destroy it.

## Node Implementation
Since there might be two different nodes in the tree that have the same board configuration, in the program, they are combined as a single node, and all nodes are allowed to link to multiple parents as well as multiple children. So a `Node` object has the following important data members:
* a `Move` object that stores either the coordinates of a move that leads to this node or an action such as "pass", "edit board" (for program flexibility) or "initialize game" (for the top node).
* two vector containers for indices to parent nodes and child nodes.
* a `Who` object (an `enum`) that specifies who played the move, either `BLACK`, `WHITE` or `NONE` (for the top node with empty board).
* a `Who` object that specifies which side is advantageous, either `BLACK`, `WHITE` or `DRAW`.
* a link to the `Board` object in the `Pool` object (see below) that specifies the stone configuration of the current node.

## Tree Implementation
`Tree` object important data members:
* a vector container for `Node` objects, new nodes can be pushed to the back, but may not be changed afterwards.
* a `Pool` object that stores every distinct board configuration (the `Board` object) that appeards in the tree.

## Board Implementation
`Board` object specifies the board configuration, not situation.
Important data members:
* a fixed size matrix container for `Who` objects, can be either `BLACK`, `WHITE` or `NONE`, storing the board configuration.

## Pool Implementation
`Pool` object important data members:
* a vector container for `Board` objects, storing every board configuration in the tree.
* a vector container for the position of each `Board` object in the `Tree` object.
* a vector container storing the order of the boards in the pool so that a board can be found quickly in the pool. This vector is updated when a new board is pushed to the pool.

## Ko Complication
If neither ko (打劫) nor upward fork is considered, the above algorithm for solving a node can be implemented easily with a recursive function `Sol Tree::solve(treeInd)`, which tries to solve a node's children one by one, by calling `solve()` itself. The end node is automatically solved when it is born (by `Tree::pass()`).

However, when a ko is found and current node linked to an upstream node called ko node (linking will update the `m_next` member of the current node, but will not update `m_last` of the ko node), `solve()` will recur infinitely. To prevent this, after a ko is linked, `solve()` will push the link destination using `Tree::m_ko_treeInds.push_back()`, and keep solving other children. If a `GOOD` child is found, then the ko link will be discarded and the current solution will be `BAD`. If not, after all children are searched, `solve()` updates `Tree::m_ko_best` using the best solved child, and sets the solution to `KO_ONLY` then return failure. When the calling `solve()` sees that failure, it will search the elements `Tree::m_ko_treeInds[]` to see if any leads to the current node and remove the matching elements. If `Tree::m_ko_treeInds` becomes empty, the solution is the opposite of `Tree::m_ko_best`. If it's not empty, `solve()` will keep solving other children. If a `GOOD` child is found, the current solution will be `BAD`. If another ko is linked, push to `Tree::m_ko_treeInds` and keep solving other children, after all children are searched, `solve()` updates `Tree::m_ko_best`, and sets the solution to `KO_ONLY` then return failure.

## GNU go
GNU go can play 5x5 to 19x19 boards. So it might be used to guess the best child for these boards. Another advantage is it might be written in c.

Other projects such as mini-go probably requires some training.
