#include<iostream>
#include "Board.h"

int main() {
	/* Basic playground testing */
    Board b = Board::blank_board();
    b.drop(0);
    b.drop(0);
    b.drop(1);
    b.drop(1);
    std::cout << b << std::endl;
    std::cout << unsigned(b.heuristic_pts()) << std::endl;
}
