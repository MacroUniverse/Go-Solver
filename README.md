# Go-Solver
Solving the game of Go by analyzing the game tree.

## Introduction
Game of Go (Chinese: weiqi 围棋) is a board game. According to game theory, with the fair komi (an integer), a draw strategy is guaranteed to exists, with an unfair komi, a winning strategy is guaranteed to exist for the advantageous side. The mind that knows these strategy is called God of Go (or god).

The fair komi is currently only known for very small boards by human, the largest board is 7 by 7, calculated by Go experts, but with no rigorous proof. The largest board proven by computer is 5 by 5.

The game tree of Go reveals all possibilities of a game. The tree is composed of connected nodes, each node represents a status of the board. The node at the top of the game tree represents an empty board, and a node at the bottom of the tree represents status of the board when the game ends. A node may connect to multiple lower nodes (child nodes), but it can only connect to one upper node (parent node). Sibling nodes are the nodes that share the same parents.

Let's assume two gods play the game starting from a given node in the tree with a given komi. According to game theory, if this is fair, the game will end with a draw, if this is unfair, the advantageous side is guaranteed to win.

The current objective of this project is to decide, for a given node and given komi, whether it is fair, and which side is advantageous (i.e. solving that node). Apparently, the closer the node is to the bottom, the easier it can be solved.

The algorithm for this is pretty straight-forward, start from any bottom node of the tree (preferable one with a short step),

1. if a node A played by X is good for X, then its parent node (played by Y) is good for X (thus bad for Y) too (no need to search any sibling nodes).
2. If a node A played by X is not good (bad or fair) for X, in order to solve the parent node, all sibling nodes must also be solved first. If at least one of them are good for X, then the parent node is good for X (rule 1). Otherwise, if a fair sibling node is found (including A), then the parent node is fair. Otherwise, the upper node is bad for X.

With these rules, the algorithm does not have to enumerate all of the downward nodes to solve a given node.

## Node Implementation
Since there might be two different nodes in the tree that have the same board configuration, in the program, they are combined as a single node, and all nodes are allowed to link to multiple parents as well as multiple children. So a `Node` object has the following important data members:
* a `Move` object that stores either the coordinates of a move that leads to this node or an action such as "pass", "edit board" (for program flexibility) or "initialize game" (for the top node).
* two vector containers for parent nodes and child nodes.
* a `Who` object (an `enum`) that specifies who played the move, either `BLACK`, `WHITE` or `NONE` (for the top node with empty board).
* a `Who` object that specifies which side is advantageous, either `BLACK`, `WHITE` or `DRAW`.
* a link to the `Board` object in the `Pool` object (see below) that specifies the stone configuration of the current node.

## Tree Implementation
`Tree` object important data members:
* a vector container for `Node` objects, new nodes can be pushed to the back, but may not be changed afterwards.
* a `Pool` object that stores every distinct board configuration (the `Board` object) that appeards in the tree.

## Board Implementation
`Board` object important data members:
* a fixed size matrix container for `Who` objects, can be either `BLACK`, `WHITE` or `NONE`, storing the board configuration.

## Pool Implementation
`Pool` object important data members:
* a vector container for `Board` objects, storing every board configuration in the tree.
* a vector container for the position of each `Board` object in the `Tree` object.
* a vector container storing the order of the boards in the pool so that a board can be found quickly in the pool. This vector is updated when a new board is pushed to the pool.
