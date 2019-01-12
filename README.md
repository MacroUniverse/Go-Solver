# Go-Solver
Solving the game of Go by analyzing the game tree.

## Introduction
Game of Go (Chinese: weiqi 围棋) is a board game. According to game theory, with the fair komi (an integer), a draw strategy is guaranteed to exists, with a unfair komi, a winning strategy is guaranteed to exist for the advantageous side. The mind that knows these strategy is called God of Go (or god).

The fair komi is currently only known for very small boards for human, the largest board is 7 by 7, calculated by Go experts, but with no rigorous proof. The largest board "solved" by computer is only 5 by 5.

The game tree of Go reveals multiple possibilities of a game, the tree is composed of connected nodes, each node represents a status of the board, the node at the top of the game tree represents an empty board, and a node at the bottom of the tree represents status of the board when the game ends.

By game theory, if to gods play the game starting from any node with any komi in the tree, if the combination of node and komi is fair, the game will end with a draw, if it is unfair, the advantageous side is guaranteed to win.

One objective of this project is to decide, for a given node and given komi, whether it is fair, and which side is advantageous (i.e. solving that node).

The algorithm for this is pretty straight-forward, start from the bottom of the tree, if a node is advantageous for black (white), then it's upper node must be advantageous for black (white) too. If all the lower nodes of a node is adverse for black (white), then the node is adverse for black (white).

With these two rules, the computer does not have to search all of the downward nodes to solve a node.
