This project is discontinued, the last working version is the last commits on Jan 28, 2019

# Go-Solver
Solving the game of Go by analyzing the game tree.

## Introduction
Game of Go (Chinese: weiqi 围棋) is a board game. According to game theory, with the fair komi (贴目, an integer), a draw strategy is guaranteed to exists, with an unfair komi, a winning strategy is guaranteed to exist for the advantageous side. The mind that knows these strategies is called God of Go (or god).

The fair komi is currently only known for very small boards by human, the largest board is 7 by 7, calculated by Go experts, but with no rigorous proof. The largest board proven by computer is 5 by 5.

The game tree of Go reveals all possibilities of a game. The tree is composed of connected nodes, each node represents a status of the board. The node at the top of the game tree represents an empty board, and a node at the bottom of the tree represents configuration of the board when the game ends. A node may connect to multiple lower nodes (child nodes), but it can only connect to one parent node (the node that leads to it). Sibling nodes are the nodes that share the same parents.

Let's assume that two gods play the game starting from a given node in the tree with a given komi. According to game theory, if the starting node is fair, the game will end with a draw, if this is unfair, the advantageous side is guaranteed to win.

The current objective of this project is to decide, for a given node and given komi, whether it is fair, and which side is advantageous (i.e. solving that node). Apparently, the closer the node is to the bottom, the easier it can be solved.

## Algorithm
The solution of a given node (good/bad/fair for the one who played this node) is the opposite of the solution of it's best (good better than fair better than bad) child played by opponent, if the best child is good (i.e. at least a good child exists) then this node is bad. Otherwise, if the best children is fair, then this node is also fair. If the best child is bad (i.e. all children are bad), this node is good.

Thus the most efficient algorithm will depend on how well another program can play Go (i.e. how well it can guess the best child, second best child etc.). For a given node, we should always solve the (guessed) best children first, then the (guessed) second best, etc, keeping track of the best child in the meanwhile to decide when to stop. This is more efficient than randomly choosing a child if the given node is bad, but is equally efficient if this node is good or fair (since we have to solve all children anyway). Since we don't know, we should always solve the (guessed) best child first.

## Computer Player Rules
Here is some rules for randomly evaluating children of a node, when another go playing program is not available.
* A dumb move is a bad move in all situation, and should never be played by the computer. Currently dumb moves include dumb eye filling and dumb big eye filling.
* An eye is a single empty point on the board surrounded by the same color of stones. 4 stones are needed for an eye not on boarder, 3 for edge, 2 for corner.
* A dumb eye filling is placing a stone in a player's own eye when it will not be destroyed by the opponent immediately. If this is allowed when enumerating child nodes, the computer will frequently commit suicide.
* A big eye is composed of two connected empty points on the board, surrounded by connected stones. It's not possible to destroy a big eye in one step.
* A dumb big eye filling is placing a stone in a player's own big eye. At the end of the game, there will be no big eye left because the opponent will eventually try to destroy it.
* Only try passing when there is no other options.
* If double passing is not allowed, it is considered a ko.

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
* If double pass happens, the second pass should not have a node, and the first pass should be linked to the node before, and these two nodes are solved by counting the scores directly. They will have the same socres and solutions.

## Territory Counting Algorithm
* assignment (认输) should never be allowed.
* double pass should only be allowed when the territory is unambiguous.

## Solution Independent of Komi
With a slightly different implementation, it's possible to solve a node without knowning the komi apriori (to distinguish, we call this scoring). Instead of deciding the solution (good/bad/fair) of a node directly, we can give it a score: the final score of the current player without komi (if two players are both god, and always choose the best scored move). It doesn't matter how this score is calculated (Chinese rule, Japanese rule, etc.). To socre a node, we must find the best scored child, and the score of the node will the the complementary (opponent's score for the same board configuration) score of it's best child. Whenever a komi is chosen, the solution for any solved node can be immediately calculated by a simple addition/subtraction.

However, this will require much more calculation because to score a node, all it's children must be scored. A compromise between solving and scoring will be to set a reasonable komi first, solve the given node and score it from the solved children, since probably not all children are solved, the score might be over estimated. If another komi is needed later, we might only need to score a few additional children of each node instead of starting all-over again or scoring all children.

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

### Basic Ko
* A basic ko is two nodes linked by a ko link and a normal link.
* If any other child of one node is good, or if the best children of both nodes are fair, then the two nodes can be solved and scored.
* If the best children of both nodes are both bad or one bad one fair, neither node will have a solution (thus, not all situations have a solution!). Who enter the ko node first will have a worse result comparing to entering second.

### Flipped Ko
* A flipped Ko is a same situation in the upstream of a node, with the colors of all stones flipped.
* A flipped ko is when the player creating a same situation for himself.
* For now I'm assuming all flipped ko are trivial and checking for non-trivial case.

### General Ko
* A ko (a ko loop with 2 or more nodes) is called a trivial ko if all it's node are solvable. Otherwise, it is called a non-trivial ko.
* There are three kinds of child: solvable child, ko link and ko child (a child that is a ko node). A forbidden child is not a child.
* A ko that includes a solvable node is a trivial ko.
* Thus all nodes in a non-trivial loop is not solvable. They can only have bad or fair solvable children.
* When a node has at least a ko link or at least a ko child, if it has a good child, it is solvable (bad).
* If a node has at least a ko link but no ko child, it is a ko node, set the score and solution of this node based on the best solved children (this score is over-estimated). If there is no child, this node is forbidden instead.
* If a node found a good ko child, but not good solvable child, then it is a bad ko child. No need to check other children.
* If a node has at least a non-good ko child but no ko link, then solve the other children one by one. If there exists a solvable child with a equal or higher score than the best ko child, then the node can be solved based on the best solvable child. Otherwise, this is a ko node, set the score and solution based on the best ko child.
* If a node has at least a ko link and at least a non-good ko child, then this node is a ko node, set the score and solution based on the score of all children except ko links.
* A ko is resolved when it's target is solved or marked as a ko node.
* A ko node with all downstream ko links resolved is a clean ko node.
* A clean ko node is eqivalent to a solvable child when solving it's parent.
* When upward fork exists, a ko link might only link to one upward branch, the solver must keep track of the current branch from the beginning of the game to decide whether this ko link points to a node in the current branch. If not, shift the ko link until it is.

### Linking to an Existing Ko Node
* shift [resolved downstream ko links to up stream] along ko loop until it targets the ko node being linked, than solve that ko node with the same algorithm as solving any node (except some downstream nodes already exist).

### SGF file output
* use Sabaki to show the output sgf file. View -> show game tree; View -> show comments
* comments: `[123]9` means the unique node number is 123, and current player will at best occupy 9 points at the end
* Node colors: green (black wins, marked `[TE]` in file); blue (white wins, marked `[IT]` in file)
* a square node means pass

## GNU go
GNU go can play 5x5 to 19x19 boards. So it might be used to guess the best child for these boards. Another advantage is it might be written in c.

Other projects such as mini-go probably requires some training.

## TODO
* all "++m_treeInd" might be wrong! some should be m_treeInd =  nnode() - 1
* a same situation in the upstream is not always a super ko! the color could be flipped for example! think about it...
* Try to prevent suicide move (no stone left)
* Try to make smarter moves firs
* Clean tree to show whinning strategy.
* After resolving a trivial ko, the downstream ko nodes should be solved as well, left only with non-trivial ko's.
* eat_pos() did not consider eating two groups at the same time
* Ko child is equivalent to a normal child when calculating parent score. So there is no need to check other children when a ko child wins.
* KO_ONLY should probably be split into KO_GOOD, KO_BAD, KO_FAIR.
* bug: ko links are not resolved in time, downstream to upstream ko links should be resolved when a node is solved.
* Even if human assistance is great, the program should be debuged in auto-solve mode.

* optimization: don't use any dynamic allocation except std::vector. use fixed-size array to store configs and other arrays.
* link object does not need so many members, all info in be compressed into Link, where the value of "type" contains all other information.
